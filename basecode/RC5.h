#ifndef _TQ_ACCOUNTSVR_RC5_INCLUDE_
#define _TQ_ACCOUNTSVR_RC5_INCLUDE_

#include "basehdr.h"
#include "memory.h"
#include "assert.h"
#include "stdlib.h"

class CRC5// : public IStreamEncryptor
{
    enum
    {
        RC5_12      = 12,
        RC5_SUB     = (RC5_12*2+2),
        RC5_16      = 16,
        RC5_KEY     = (RC5_16/4),
        RC5_32      = 32,
    };
public:
    CRC5(void);
	~CRC5();
public:
    virtual bool SetKey(const uint8 *buf, uint32 nLen);
    virtual void Release();
    virtual bool Encrypt(uint8 *buf, uint32 &nLen);
    virtual bool Decrypt(uint8 *buf, uint32 nLen);
    //virtual IEncryptor* Duplicate(void);
    virtual uint32 PadSize()            { return 8; }
    virtual uint32 GetPadSize( uint32 nLen);

public:
	virtual void ChangeCode( DWORD dwData );

private:
    CRC5(const CRC5 &rhs);
    CRC5& operator=(const CRC5 &rhs);

private:
    unsigned long	m_bufKey[RC5_KEY];
    unsigned long	m_bufSub[RC5_SUB];

    uint8            m_cbKey[RC5_16+1];
};

#endif  //_TQ_ACCOUNTSVR_GAMESECURITY_RC5_INCLUDE_