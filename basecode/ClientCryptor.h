#ifndef ENCRYPT_CLIENT_H__
#define ENCRYPT_CLIENT_H__

#include "basecode/basehdr.h"
#include <string.h>

class CEncryptClient
{
public:
    CEncryptClient( uint8 a1, uint8 b1, uint8 c1, uint8 fst1,
				uint8 a2, uint8 b2, uint8 c2, uint8 fst2, bool bIsConquest=false );
	virtual ~CEncryptClient() {}
	void Init();

public:
	virtual bool SetKey(const uint8 *buf, size_t nLen);
	virtual void Release(); 
	virtual bool Encrypt(uint8 *bufMsg, size_t &nLen);
	virtual bool Decrypt(uint8 *bufMsg, size_t nLen);
	virtual size_t PadSize()            { return 1; }
	virtual size_t GetPadSize(size_t nLen) { return nLen; }

public:
	virtual void ChangeCode(DWORD dwData);
	virtual void ChangeCode(const char* pszKey);
	virtual void Reset();

protected:
	bool internal_encrypt( uint8 *bufMsg, size_t &nLen, int &nPos1, int &nPos2 );
	CEncryptClient( const CEncryptClient &rhs );
	CEncryptClient& operator=( const CEncryptClient &rhs );

private:
	unsigned char m_bufEncrypt1[256];
	unsigned char m_bufEncrypt2[256];

	int		    m_nPos1;
	int		    m_nPos2;    // <== 加密
	
	int         m_nPos3;
	int         m_nPos4;    // <== 解密

	uint8       m_cA1;
	uint8       m_cB1;
	uint8       m_cC1;
	uint8       m_cFst1;
	uint8       m_cA2;
	uint8       m_cB2;
	uint8       m_cC2;
	uint8       m_cFst2;

    bool        m_bIsConquest;      //是否为征服的算法
};

#endif  //_TQ_ACCOUNTSVR_GAMESECURITY_CLIENT_INCLUDED_