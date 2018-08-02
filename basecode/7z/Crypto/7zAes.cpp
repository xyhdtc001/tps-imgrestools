
#include "7z/Crypto/Sha256.h"
#include "7z/Crypto/Synchronization.h"

#include "7z/Crypto/7zAes.h"

using namespace NWindows;

namespace NCrypto {
namespace NSevenZ {

bool CKeyInfo::IsEqualTo(const CKeyInfo &a) const
{
  if (SaltSize != a.SaltSize || NumCyclesPower != a.NumCyclesPower)
    return false;
  for (UInt32 i = 0; i < SaltSize; i++)
    if (Salt[i] != a.Salt[i])
      return false;
  return (Password == a.Password);
}

void CKeyInfo::CalculateDigest()
{
  if (NumCyclesPower == 0x3F)
  {
    UInt32 pos;
    for (pos = 0; pos < SaltSize; pos++)
      Key[pos] = Salt[pos];
    for (UInt32 i = 0; i < Password.GetCapacity() && pos < kKeySize; i++)
      Key[pos++] = Password[i];
    for (; pos < kKeySize; pos++)
      Key[pos] = 0;
  }
  else
  {
    CSha256 sha;
    Sha256_Init(&sha);
    const UInt64 numRounds = (UInt64)1 << NumCyclesPower;
    Byte temp[8] = { 0,0,0,0,0,0,0,0 };
    for (UInt64 round = 0; round < numRounds; round++)
    {
      Sha256_Update(&sha, Salt, (size_t)SaltSize);
      Sha256_Update(&sha, Password, Password.GetCapacity());
      Sha256_Update(&sha, temp, 8);
      for (int i = 0; i < 8; i++)
        if (++(temp[i]) != 0)
          break;
    }
    Sha256_Final(&sha, Key);
  }
}

bool CKeyInfoCache::Find(CKeyInfo &key)
{
	KeyVector::iterator it = Keys.begin();
	for (; it != Keys.end(); ++it)
	{
		const CKeyInfo &cached = *it;
		if (key.IsEqualTo(cached))
		{
			for (int j = 0; j < kKeySize; j++)
				key.Key[j] = cached.Key[j];
			if (it != Keys.begin())
			{
				Keys.erase(it);
				Keys.push_front(cached);
			}
			return true;
		}
	}
	return false;
}

void CKeyInfoCache::Add(CKeyInfo &key)
{
  if (Find(key))
    return;
  if ( (int)Keys.size() >= Size )
    Keys.pop_back();
  Keys.push_front(key);
}

static CKeyInfoCache g_GlobalKeyCache(32);
static NSynchronization::CCriticalSection g_GlobalKeyCacheCriticalSection;

CBase::CBase():
  _cachedKeys(16),
  _ivSize(0)
{
  for (int i = 0; i < sizeof(_iv); i++)
    _iv[i] = 0;
}

void CBase::CalculateDigest()
{
  NSynchronization::CCriticalSectionLock lock(g_GlobalKeyCacheCriticalSection);
  if (_cachedKeys.Find(_key))
    g_GlobalKeyCache.Add(_key);
  else
  {
    if (!g_GlobalKeyCache.Find(_key))
    {
      _key.CalculateDigest();
      g_GlobalKeyCache.Add(_key);
    }
    _cachedKeys.Add(_key);
  }
}

STDMETHODIMP CDecoder::SetDecoderProperties2(const Byte *data, UInt32 size)
{
  _key.Init();
  UInt32 i;
  for (i = 0; i < sizeof(_iv); i++)
    _iv[i] = 0;
  if (size == 0)
    return S_OK;
  UInt32 pos = 0;
  Byte firstByte = data[pos++];

  _key.NumCyclesPower = firstByte & 0x3F;
  if ((firstByte & 0xC0) == 0)
    return S_OK;
  _key.SaltSize = (firstByte >> 7) & 1;
  UInt32 ivSize = (firstByte >> 6) & 1;

  if (pos >= size)
    return E_INVALIDARG;
  Byte secondByte = data[pos++];
  
  _key.SaltSize += (secondByte >> 4);
  ivSize += (secondByte & 0x0F);
  
  if (pos + _key.SaltSize + ivSize > size)
    return E_INVALIDARG;
  for (i = 0; i < _key.SaltSize; i++)
    _key.Salt[i] = data[pos++];
  for (i = 0; i < ivSize; i++)
    _iv[i] = data[pos++];
  return (_key.NumCyclesPower <= 24) ? S_OK :  E_NOTIMPL;
}


CBaseCoder::CBaseCoder() :
_aesFilter(NULL)
{

}


STDMETHODIMP CBaseCoder::CryptoSetPassword(const Byte *data, UInt32 size)
{
  _key.Password.SetCapacity((size_t)size);
  memcpy(_key.Password, data, (size_t)size);
  return S_OK;
}

STDMETHODIMP CBaseCoder::Init()
{
  CalculateDigest();
  if (_aesFilter == 0)
  {
    RINOK(CreateFilter());
  }
  RINOK(_aesFilter->SetKey(_key.Key, sizeof(_key.Key)));
  RINOK(_aesFilter->SetInitVector(_iv, sizeof(_iv)));
  return S_OK;
}

STDMETHODIMP_(UInt32) CBaseCoder::Filter(Byte *data, UInt32 size)
{
  return _aesFilter->Filter(data, size);
}

HRESULT CDecoder::CreateFilter()
{
  _aesFilter = new CAesCbcDecoder;
  return S_OK;
}

}}
