#include "basecode/ClientCryptor.h"
#include <memory.h>
#include <assert.h>

CEncryptClient::CEncryptClient( uint8 a1, uint8 b1, uint8 c1, uint8 fst1, uint8 a2, uint8 b2, uint8 c2, uint8 fst2, bool bIsConquest )
: m_cA1(a1), m_cB1(b1), m_cC1(c1), m_cFst1(fst1)
, m_cA2(a2), m_cB2(b2), m_cC2(c2), m_cFst2(fst2)
, m_bIsConquest(bIsConquest)
, m_nPos1(0), m_nPos2(0)
, m_nPos3(0), m_nPos4(0)
{
	memset( m_bufEncrypt1, 0, sizeof(m_bufEncrypt1) );
	memset( m_bufEncrypt2, 0, sizeof(m_bufEncrypt2) );

	//* 与下面ELSE算法相同，只为加强复杂性：)
	unsigned char	nCode = fst1;
	int i;
	for ( i = 0; i < 256; i++ )
	{
		m_bufEncrypt1[i] = nCode;
		int	nTemp = (a1*nCode) % 256;
		nCode = (unsigned char)((c1 + nTemp*nCode + b1*nCode) % 256);
	}
	assert( nCode == fst1 );
	nCode = fst2;
	for ( i = 0; i < 256; i++ )
	{
		m_bufEncrypt2[i] = nCode;
		int	nTemp = a2*nCode;
		nCode = (unsigned char)(((b2 + nTemp)*nCode + c2) & 0xFF);
	}
	assert( nCode == fst2 );

}

CEncryptClient::CEncryptClient( const CEncryptClient &rhs )
{
    memset( m_bufEncrypt1, 0, sizeof(m_bufEncrypt1) );
    memset( m_bufEncrypt2, 0, sizeof(m_bufEncrypt2) );
    memcpy( m_bufEncrypt1, rhs.m_bufEncrypt1, sizeof(m_bufEncrypt1) );
    memcpy( m_bufEncrypt2, rhs.m_bufEncrypt2, sizeof(m_bufEncrypt2) );
    m_nPos1 = rhs.m_nPos1;
    m_nPos2 = rhs.m_nPos2;
    m_nPos3 = rhs.m_nPos3;
    m_nPos4 = rhs.m_nPos4;
    m_cA1   = rhs.m_cA1;
    m_cB1   = rhs.m_cB1;
    m_cC1   = rhs.m_cC1;
    m_cFst1 = rhs.m_cFst1;
    m_cA2   = rhs.m_cA2;
    m_cB2   = rhs.m_cB2;
    m_cC2   = rhs.m_cC2;
    m_cFst2 = rhs.m_cFst2;
    m_bIsConquest =   rhs.m_bIsConquest;
}

void CEncryptClient::Init()
{
    m_nPos1 = 0;
    m_nPos2 = 0; 
    m_nPos3 = 0;
    m_nPos4 = 0;
}

bool CEncryptClient::SetKey( const uint8 *buf, size_t nLen )
{
	//UNUSED(buf);
    //UNUSED(nLen);
    return true;
}

void CEncryptClient::Release() 
{ 
    delete this;
}

bool CEncryptClient::internal_encrypt( uint8 *bufMsg, size_t &nLen, int &nPos1, int &nPos2 )
{
	bool result = true;
	bool bMove = true;
	int		nOldPos1 = nPos1;
	int		nOldPos2 = nPos2;
	for(size_t i = 0; i < nLen; i++)
	{
		bufMsg[i] ^= m_bufEncrypt1[nPos1];
		bufMsg[i] ^= m_bufEncrypt2[nPos2];
		if(++nPos1 >= 256)
		{
			nPos1 = 0;
			if(++nPos2 >= 256)
				nPos2 = 0;
		}
		//@@@	assert(nPos1 >=0 && nPos1 < 256);
		//@@@	assert(nPos2 >=0 && nPos2 < 256);
		if (m_bIsConquest)
		{
			int a = (bufMsg[i]&0x0f)*0x10;
			int b = (bufMsg[i]&0xf0)/0x10;
			bufMsg[i] = (unsigned char)((a + b) ^ 0xab);
		}
	}

	if(!bMove)
	{
		// 恢复指针
		nPos1 = nOldPos1;
		nPos2 = nOldPos2;
	}
	return result;
}

bool CEncryptClient::Encrypt( uint8 *bufMsg, size_t &nLen )
{
    return this->internal_encrypt( bufMsg, nLen, m_nPos1, m_nPos2 );
}

bool CEncryptClient::Decrypt( uint8 *bufMsg, size_t nLen )
{
    return this->internal_encrypt( bufMsg, nLen, m_nPos3, m_nPos4 );
}

void CEncryptClient::ChangeCode( DWORD dwData )
{
	DWORD	dwData2 = dwData*dwData;
	for(int i = 0; i < 256; i += 4)
	{
		*(DWORD*)(&m_bufEncrypt1[i]) ^= dwData;
		*(DWORD*)(&m_bufEncrypt2[i]) ^= dwData2;
	}
}

void CEncryptClient::ChangeCode( const char* pszKey )
{
    //UNUSED(pszKey); 
}

void CEncryptClient::Reset()
{
    this->Init();
}

