#include "basecode/RC5.h"
#include "basecode/basefunc.h"

#ifdef GUOWEI_ONLINE
    const unsigned long	RC5_PW32	= 0xA7D35183;
    const unsigned long	RC5_QW32	= 0xCE3779A9;
#else
    const unsigned long	RC5_PW32	= 0xB7E15163;
    const unsigned long	RC5_QW32	= 0x9E3779B9;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long rotate_left(unsigned long nData, unsigned long nCount)
{
    nCount %= 32;

    unsigned long	nHigh = nData >> (32-nCount);
    return (nData << nCount) | nHigh;
}

unsigned long	rotate_right(unsigned long nData, unsigned long nCount)
{
    nCount %= 32;

    unsigned long	nLow = nData << (32-nCount);
    return (nData >> nCount) | nLow;
}

//////////////////////////////////////////////////////////////////////////
//CRC5
CRC5::CRC5(void)
{
    memset(m_bufKey, 0, sizeof(m_bufKey));
    memset(m_bufSub, 0, sizeof(m_bufSub));
    memset(m_cbKey, 0, sizeof(m_cbKey));
}

CRC5::~CRC5(void)
{
}

void CRC5::Release()
{
    delete this;
}

bool CRC5::SetKey( const uint8 *buf, uint32 nLen )
{
    assert( nLen == RC5_16 );
    // 保存m_bufKey
    memcpy( m_bufKey, buf, RC5_16 );
    memcpy( m_cbKey, buf, RC5_16 );

    // 初始化m_bufSub
    m_bufSub[0] = RC5_PW32;
    int i = 0;
    for ( i = 1; i < RC5_SUB; i++ )
    {
        m_bufSub[i] = m_bufSub[i-1] + RC5_QW32;
    }

    // 生成m_bufSub
    int				/*i,*/ j;
    unsigned long	x, y;
    i = j = x = y = 0;
    for(int k = 0; k < 3 * __max(RC5_KEY, RC5_SUB); k++)
    {
        m_bufSub[i] = rotate_left((m_bufSub[i] + x + y), 3);
        x = m_bufSub[i];
        i = (i + 1) % RC5_SUB;
        m_bufKey[j] = rotate_left((m_bufKey[j] + x + y), (x + y));
        y = m_bufKey[j];
        j = (j + 1) % RC5_KEY;
    }

    return true;
}

uint32 CRC5::GetPadSize( uint32 nLen )
{
    uint32 nLeft = nLen % this->PadSize();
    return nLen + ((0 == nLeft) ? 0 : (this->PadSize() - nLeft));
}

/*
IEncryptor* CRC5::Duplicate(void)
{
    IEncryptor *pRC5 = new CRC5;
    if (NULL != pRC5)
    {
        if (!pRC5->SetKey(m_cbKey, RC5_16))
        {
            pRC5->Release();
            pRC5 = NULL;
        }
    }
    return pRC5;
}
*/

bool CRC5::Encrypt(uint8 *buf, uint32 &nLen)
{
    if (nLen <= 0) 
    {
        return false;
    }

    uint32 nNewLen = this->GetPadSize(nLen);
    if (nLen < nNewLen)
    {
        nNewLen -= this->PadSize();
        if (nNewLen <= 0)
        {
            return false;
        }
    }
    nLen = nNewLen;

    //加密过程
    uint32 nCount = nNewLen / this->PadSize();
    unsigned long* bufData = (unsigned long*)buf;
    for(uint32 k = 0; k < nCount; k++)
    {
        unsigned long	a = bufData[2*k];
        unsigned long	b = bufData[2*k + 1];

        // 加密a b
        unsigned long	le	= a + m_bufSub[0];
        unsigned long	re	= b + m_bufSub[1];
        for(int i = 1; i <= RC5_12; i++)
        {
            le = rotate_left((le ^ re), re) + m_bufSub[2*i];
            re = rotate_left((re ^ le), le) + m_bufSub[2*i + 1];
        }

        bufData[2*k]		= le;
        bufData[2*k + 1]	= re;
    }
    
    return true;
}

bool CRC5::Decrypt(uint8 *buf, uint32 nLen)
{
    if ( (nLen % this->PadSize()) != 0)
    {
        return false;
    }

    uint32 nCount = nLen / this->PadSize();
    unsigned long* bufData = (unsigned long*)buf;
    for(uint32 k = 0; k < nCount; k++)
    {
        // 解密 ld rd
        unsigned long	ld	= bufData[2*k];
        unsigned long	rd	= bufData[2*k + 1];
        for(int i = RC5_12; i >= 1; i--)
        {
            rd = rotate_right((rd - m_bufSub[2*i + 1]),  ld) ^ ld;
            ld = rotate_right((ld - m_bufSub[2*i]),  rd) ^ rd;
        }

        unsigned long	b = rd - m_bufSub[1];
        unsigned long	a = ld - m_bufSub[0];

        bufData[2*k]		= a;
        bufData[2*k + 1]	= b;
    }
    
    return true;
}


void CRC5::ChangeCode( DWORD dwData )
{
#ifdef WINDOWS
    ::srand(dwData);
#else
    my_win_srand(dwData);
#endif
    uint8 szKeyBuf[RC5_16] = "";
    for (uint32 i = 0; i < sizeof(szKeyBuf); ++i)
    {
#ifdef WINDOWS
        szKeyBuf[i] = ::rand() % 256;
#else
        szKeyBuf[i] = ::my_win_rand() % 256;
#endif
    }
    this->SetKey(szKeyBuf, RC5_16);
}
