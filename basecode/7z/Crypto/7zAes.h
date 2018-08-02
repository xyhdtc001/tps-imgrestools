// 7zAes.h

#ifndef __CRYPTO_7Z_AES_H
#define __CRYPTO_7Z_AES_H

#include "Buffer.h"
#include <deque>

#include "7z/Crypto/MyAes.h"

namespace NCrypto {
namespace NSevenZ {

const int kKeySize = 32;

class CKeyInfo
{
public:
  int NumCyclesPower;
  UInt32 SaltSize;
  Byte Salt[16];
  CByteBuffer Password;
  Byte Key[kKeySize];

  bool IsEqualTo(const CKeyInfo &a) const;
  void CalculateDigest();

  CKeyInfo() { Init(); }
  void Init()
  {
    NumCyclesPower = 0;
    SaltSize = 0;
    for (int i = 0; i < sizeof(Salt); i++)
      Salt[i] = 0;
  }
};

class CKeyInfoCache
{
  int Size;

  typedef std::deque<CKeyInfo> KeyVector;
  KeyVector Keys;
public:
  CKeyInfoCache(int size): Size(size) {}
  bool Find(CKeyInfo &key);
  // HRESULT Calculate(CKeyInfo &key);
  void Add(CKeyInfo &key);
};

class CBase
{
  CKeyInfoCache _cachedKeys;
protected:
  CKeyInfo _key;
  Byte _iv[16];
  UInt32 _ivSize;
  void CalculateDigest();
  CBase();
};

class CBaseCoder : public CBase
{
public:
  CBaseCoder();
protected:
  CAesCbcCoder* _aesFilter;

  virtual HRESULT CreateFilter() = 0;
  #ifndef CRYPTO_AES
  HRESULT CreateFilterFromDLL(REFCLSID clsID);
  #endif
public:
  STDMETHOD(Init)();
  STDMETHOD_(UInt32, Filter)(Byte *data, UInt32 size);
  
  STDMETHOD(CryptoSetPassword)(const Byte *data, UInt32 size);
};

class CDecoder : public CBaseCoder
{
  virtual HRESULT CreateFilter();
public:
  STDMETHOD(SetDecoderProperties2)(const Byte *data, UInt32 size);
};

}}

#endif
