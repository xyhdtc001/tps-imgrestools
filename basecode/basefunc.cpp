#include <assert.h>

#include "windows.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "basecode/basehdr.h"
#include "basecode/BaseFunc.h"
#include "basecode/BaseLog.h"

#include "Shlobj.h"
#include "Shlwapi.h"
#include "olectl.h"

#if !defined WINDOWS
#include <sys/time.h>
#include <time.h>
#endif

bool ascBcd( const char* ascstr,unsigned char* bcdstr,int bcdlen )
{
    int i, j, m, n;

    j = bcdlen*2-1;
    for( i=j/2;i>=0; i-- )
    {
        if( j<=0 )
            return false;
        else
        {
            m = AsccharToInt( ascstr[j-1] );
            n = AsccharToInt( ascstr[j] );
            if( m==-1 || n==-1 )
                return false; 
            bcdstr[i] = (m<<4)|n;
            j -= 2;
        }
    }
    return true;
}


void bcdAsc( char *cbuf, const unsigned char *ubuf, int len )
{
	int i;
	for ( i=0; i<len*2; i++ )
	{
		if ( i%2==0 )
			cbuf[i] = ubuf[i/2]/16;
		else
			cbuf[i] = ubuf[i/2]%16;
		if ( cbuf[i] > 9 ) 
			cbuf[i] += ('A'-10);
		else
			cbuf[i] += '0'; 
	}
	cbuf[i]='\0';
}


char* DelSpace( char* strConvert)
{
	char* strNoSpace = strConvert;
	char* lpStrEnd;
	while( ' ' == *strNoSpace )
		strNoSpace++;
	if (0 == strlen(strNoSpace))		//如果为空字符串返回
		return strNoSpace;
	lpStrEnd = strNoSpace+strlen(strNoSpace)-1;		//指向最后一个字符
	while( ' ' == *lpStrEnd )
		*(lpStrEnd--) =  '\0';
	return strNoSpace;
}

int HexToint(char* hexstr)
{
	int iret = 0;
	int tmp;
	char* phex;
	unsigned int i;
	phex = hexstr + strlen(hexstr) - 1;	//指向末尾数据
	if ( strlen(hexstr) >= sizeof(int)*2 )
		return -1;
	for( i=0; i<strlen(hexstr); i++ )
	{
		if ( (*phex>='a')&&(*phex<='h') )
			tmp = *phex-'a'+10;
		else if((*phex>='A')&&(*phex<='H'))
			tmp = *phex-'A'+10;
		else if( (*phex>='0')&&(*phex<='9') )
			tmp = *phex-'0';
		else
			return -1;
		tmp = tmp<<(4*i);
		iret+=tmp;
		phex--;
	}
	return iret;
}

int AsccharToInt( unsigned char asc )
{
    if( asc<='9' && asc>='0' )
        return( asc-'0' );
    else if( asc<='F' && asc>='A' )
        return( asc-'A'+0x0A );
    else if( asc<='f' && asc>='a' )
        return( asc-'a'+0x0A );
    else
        return( -1 );
}

void AscstrToBcdstr( unsigned char *ascstr, unsigned char *bcdstr, int asclen )
{
    int i, j;

    j = asclen-1;
    for( i=j/2;i>=0; i-- )
    {
        if( j<=0 )
            bcdstr[i] = 0x0F & AsccharToInt(ascstr[j]);
        else
        {
            bcdstr[i] = (AsccharToInt(ascstr[j-1])<<4)|AsccharToInt(ascstr[j]);
            j -= 2;
        }
    }
}


int BcdcharToint( unsigned char bcdchar )
{
    return( ((bcdchar&0xf0)>>4)*10+(bcdchar&0x0f) );
}

unsigned short checksum( unsigned short *buffer,int size ) 
{ 
	unsigned long cksum=0;
	while ( size > 1 )
	{
		cksum += *buffer++;
		size  -= sizeof(unsigned short);   
	}
	if ( size )
		cksum += *(unsigned char*)buffer;
	cksum  = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16); 
	return (unsigned short)(~cksum); 
}

double	GetDistanceBy3D(float fX0, float fY0, float fZ0, float fX1, float fY1, float fZ1)
{
	return sqrt((fX0 - fX1) * (fX0 - fX1) + (fY0 - fY1) * (fY0 - fY1) + (fZ0 - fZ1) * (fZ0 - fZ1));
}


Data GetCurrentExeDir()
{
#if defined WINDOWS
	static const int MAX_STR_NAME = 1024;
	char szFullPath[MAX_STR_NAME] = "";
	::GetModuleFileName(NULL, szFullPath, MAX_STR_NAME);

	char szDrive[MAX_STR_NAME] = "";
	char szDir[MAX_STR_NAME] = "";
	char szFileName[MAX_STR_NAME] = "";
	char szExt[MAX_STR_NAME] = "";
	_splitpath( szFullPath, szDrive, szDir, szFileName, szExt );
	char szReturn[MAX_STR_NAME] = "";
	strcpy( szReturn, szDrive );
	strcat( szReturn, szDir );
	return szReturn;
#else
    return "";
#endif
}


#define OGRE_GET16BITS(d) (*((const uint8 *) (d)) + (*((const uint8 *) (d+1))<<8))
uint32 fastHash( const char * data, int len, uint32 hashSoFar /*= 0*/ )
{
	uint32 hash;
	uint32 tmp;
	int rem;

	if (hashSoFar)
		hash = hashSoFar;
	else
		hash = len;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (;len > 0; len--) {
		hash  += OGRE_GET16BITS (data);
		tmp    = (OGRE_GET16BITS (data+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		data  += 2*sizeof (uint16);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
		case 3: hash += OGRE_GET16BITS (data);
			hash ^= hash << 16;
			hash ^= data[sizeof (uint16)] << 18;
			hash += hash >> 11;
			break;
		case 2: hash += OGRE_GET16BITS (data);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1: hash += *data;
			hash ^= hash << 10;
			hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}


int
compareNoCase( const char *str1, const char *str2, int len )
{
	for ( int i=0; i<len; i++ )
	{
		if ( toupper(*str1) < toupper(*str2) )
		{
			return -1;
		}
		if ( toupper(*str1) > toupper(*str2) )
		{
			return 1;
		}
		str1++;
		str2++;
	}
	return 0;
}


bool
equalNoCase( const char *str1, const char *str2 )
{
	uint32 len;
	if ( (len=(uint32)strlen(str1)) != (uint32)strlen(str2) )
		return false;
	for ( uint32 i=0; i<len; i++ )
	{
		if ( toupper(*str1) != toupper(*str2) )
			return false;
		str1++;
		str2++;
	}
	return true;
}



double UInt64ToDouble( uint64 uValue )
{
	int64 n64Value = (int64)uValue;
	double dbValue = (double)n64Value;
	return dbValue;
}


void vsleep( long millsec )
{
#if defined WINDOWS
    static HANDLE event = NULL;
	if ( event == NULL )
		event = CreateEvent( NULL, FALSE, FALSE, NULL );
	WaitForSingleObject( event, millsec );
#else
	//ASSERT( false );
    usleep(millsec*1000);
#endif
};

int strnatcmp0( const char* a, const char* b, int fold_case )
{
	int ai, bi;
	char ca, cb;
	//int fractional, result;

	ai = bi = 0;
	while ( 1 )
	{
		ca = a[ai]; cb = b[bi];
//       /* skip over leading spaces or zeros */
//       while ( isspace(ca) )
//          ca = a[++ai];
//       while ( isspace(cb) )
//          cb = b[++bi];
//       /* process run of digits */
//       if (isdigit(ca)  &&  isdigit(cb)) {
//          fractional = (ca == '0' || cb == '0');
//          if (fractional) {
//             if ((result = compare_left(a+ai, b+bi)) != 0)
//                return result;
//          } else {
//             if ((result = compare_right(a+ai, b+bi)) != 0)
//                return result;
//          }
//       }

      if (!ca && !cb) {
         /* The strings compare the same.  Perhaps the caller
         will want to call strcmp to break the tie. */
         return 0;
      }

      if (fold_case) {
         ca = toupper(ca);
         cb = toupper(cb);
      }

      if (ca < cb)
         return -1;
      else if (ca > cb)
         return +1;

      ++ai; ++bi;
   }
}


int compare_left( const char* a, const char* b )
{
   /* Compare two left-aligned numbers: the first to have a
   different value wins. */
   for ( ;; a++, b++ ) 
   {
      if ( !isdigit(*a) && !isdigit(*b) )
         return 0;
      else if ( !isdigit(*a) )
         return -1;
      else if ( !isdigit(*b) )
         return +1;
      else if ( *a < *b )
         return -1;
      else if ( *a > *b )
         return +1;
   }
   return 0;
}


int compare_right( const char* a, const char* b )
{
   int bias = 0;

   /* The longest run of digits wins.  That aside, the greatest
   value wins, but we can't know that it will until we've scanned
   both numbers to know that they have the same magnitude, so we
   remember it in BIAS. */
   for (;; a++, b++)
   {
      if ( !isdigit(*a)  &&  !isdigit(*b) )
         return bias;
      else if (!isdigit(*a))
         return -1;
      else if (!isdigit(*b))
         return +1;
      else if (*a < *b) {
         if (!bias)
            bias = -1;
      } else if (*a > *b) {
         if (!bias)
            bias = +1;
      } else if (!*a  &&  !*b)
         return bias;
   }
   return 0;
}

void expandEscape(char *dest, const char *src)
{
	uint8 c;
	while( (c = (uint8)*src++) != 0 )
	{
		if(c == '\"')
		{
			*dest++ = '\\';
			*dest++ = '\"';
		}
		else if(c == '\\')
		{
			*dest++ = '\\';
			*dest++ = '\\';
		}
		else if(c == '\r')
		{
			*dest++ = '\\';
			*dest++ = 'r';
		}
		else if(c == '\n')
		{
			*dest++ = '\\';
			*dest++ = 'n';
		}
		else if(c == '\t')
		{
			*dest++ = '\\';
			*dest++ = 't';
		}
		else if(c == '\'')
		{
			*dest++ = '\\';
			*dest++ = '\'';
		}
		else if((c >= 1 && c <= 7) ||
			(c >= 11 && c <= 12) ||
			(c >= 14 && c <= 15))
		{
			/*  Remap around: \b = 0x8, \t = 0x9, \n = 0xa, \r = 0xd */
			static uint8 expandRemap[15] = { 0x0,
				0x0,
				0x1,
				0x2,
				0x3,
				0x4,
				0x5,
				0x6,
				0x0,
				0x0,
				0x0,
				0x7,
				0x8,
				0x0,
				0x9 };

			*dest++ = '\\';
			*dest++ = 'c';
			if(c == 15)
				*dest++ = 'r';
			else if(c == 16)
				*dest++ = 'p';
			else if(c == 17)
				*dest++ = 'o';
			else
				*dest++ = expandRemap[c] + '0';
		}
		else if(c < 32)
		{
			*dest++ = '\\';
			*dest++ = 'x';
			int32 dig1 = c >> 4;
			int32 dig2 = c & 0xf;
			if(dig1 < 10)
				dig1 += '0';
			else
				dig1 += 'A' - 10;
			if(dig2 < 10)
				dig2 += '0';
			else
				dig2 += 'A' - 10;
			*dest++ = dig1;
			*dest++ = dig2;
		}
		else
			*dest++ = c;
	}
	*dest = '\0';
}

int getHexDigit( char c )
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if(c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}


bool collapseEscape(char *buf)
{
	int32 len = (int)strlen(buf) + 1;
	for ( int32 i = 0; i < len; )
	{
		if(buf[i] == '\\')
		{
			if(buf[i+1] == 'x')
			{
				int32 dig1 = getHexDigit(buf[i+2]);
				if(dig1 == -1)
					return false;

				int32 dig2 = getHexDigit(buf[i+3]);
				if(dig2 == -1)
					return false;
				buf[i] = dig1 * 16 + dig2;
				memmove(buf + i + 1, buf + i + 4, len - i - 3);
				len -= 3;
				i++;
			}
			else if(buf[i+1] == 'c')
			{
				/*  Remap around: \b = 0x8, \t = 0x9, \n = 0xa, \r = 0xd */
				static uint8 collapseRemap[10] = { 0x1,
					0x2,
					0x3,
					0x4,
					0x5,
					0x6,
					0x7,
					0xb,
					0xc,
					0xe };

				if(buf[i+2] == 'r')
					buf[i] = 15;
				else if(buf[i+2] == 'p')
					buf[i] = 16;
				else if(buf[i+2] == 'o')
					buf[i] = 17;
				else
				{
					int dig1 = buf[i+2] - '0';
					if(dig1 < 0 || dig1 > 9)
						return false;
					buf[i] = collapseRemap[dig1];
				}
				// Make sure we don't put 0x1 at the beginning of the string.
				if ((buf[i] == 0x1) && (i == 0))
				{
					buf[i] = 0x2;
					buf[i+1] = 0x1;
					memmove(buf + i + 2, buf + i + 3, len - i - 1);
					len -= 1;
				}
				else
				{
					memmove(buf + i + 1, buf + i + 3, len - i - 2);
					len -= 2;
				}
				i++;
			}
			else
			{
				buf[i] = charConv(buf[i+1]);
				memmove(buf + i + 1, buf + i + 2, len - i - 1);
				len--;
				i++;
			}
		}
		else
			i++;
	}
	return true;
}

int charConv(int in)
{
	switch(in)
	{
	case 'r':
		return '\r';
	case 'n':
		return '\n';
	case 't':
		return '\t';
	default:
		return in;
	}
}


typedef unsigned char BYTE;

inline BYTE toHex(const BYTE &x)
{
	return x > 9 ? x -10 + 'A': x + '0';
}

inline BYTE fromHex(const BYTE &x)
{
	return isdigit(x) ? x-'0' : x-'A'+10;
}


std::string URLEncode( const std::string &sIn )
{
	String sOut;
	for( size_t ix = 0; ix < sIn.size(); ix++ )
	{      
		BYTE buf[4];
		memset( buf, 0, 4 );
		if( isalnum( (BYTE)sIn[ix] ) )
		{      
			buf[0] = sIn[ix];
		}
		//else if ( isspace( (BYTE)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
		//{
		//    buf[0] = '+';
		//}
		else
		{
			buf[0] = '%';
			buf[1] = toHex( (BYTE)sIn[ix] >> 4 );
			buf[2] = toHex( (BYTE)sIn[ix] % 16);
		}
		sOut += (char *)buf;
	}
	return sOut.c_str();
};


uint32 getSysMillsec()
{
#if defined WINDOWS
#if defined METRO
	return 0;
#else
	return GetTickCount();
#endif
#else
	struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec/1000);
#endif
}


uint32 getSysSecond()
{
#if defined WINDOWS
	return GetTickCount()/1000;
#else
	return getSysMillsec()/1000;
#endif
}


void AssertMsg(bool bOK,const char* pszExpr,const char* pszFile,int nLine,const char* fmt,...)
{
	if (!bOK)
	{
		const int len = 1024;
		char buffer[len];

		if (fmt)
		{
			va_list args;
			va_start( args, fmt );
			vsprintf( buffer, fmt, args );
			va_end(args);
		}else{
			buffer[0] = '\0';
		}

		char szNum[16];
		itoa(nLine, szNum, 10);

		std::string strWhat = "Assert Occur : \n";
		strWhat += pszFile;
		strWhat += "(";
		strWhat += szNum;
		strWhat += ")\nError Desc : ";
		strWhat += buffer;
		strWhat += "\n";
		LogOut( MAIN, LOG_ERR, "assert: %d", strWhat.c_str() );
		assert(false);
	}
}

#if defined WINDOWS
	#pragma comment(lib, "winmm.lib")
#endif

static uint32 g_timeMillsec = 0;
void resetTimeMillsec()
{
// 	LARGE_INTEGER perfFreq;
// 	LARGE_INTEGER perfTime;
// 	BOOL bVal = 
// 	if ( QueryPerformanceFrequency(&perfFreq) )
// 	{
// 		QueryPerformanceCounter( &perfTime );
// 		g_timeMillsec = (perfTime.QuadPart*1000)/perfFreq.QuadPart;
// 	}
// 	else
// 	{
		g_timeMillsec = timeGetTime();
//	}
}

uint32 getTimeMillsec()
{
#if defined METRO
	return 0;
#else
	return g_timeMillsec;
#endif
}

uint32 getSysTimeMillsec()
{
	return timeGetTime();
}

/// ios srand & rand
static int mySeed = 1;
void my_gcc_srand(int i)
{
    mySeed = i;
}
int my_gcc_rand()
{
    long quotient, remainder, t;
    quotient = mySeed/127773L;
    remainder = mySeed%127773L;
    t = 16807L*remainder - 2836L*quotient;
    if (t<=0)
        t = 0x7FFFFFFFL;
    mySeed = t;
    return (mySeed % ((unsigned long)0x7fffffff+1));
}
// windows srand & rand
static int myWinSeed = 1;
void my_win_srand(int i)
{
    myWinSeed = i;
}
int my_win_rand()
{
    return ( ((myWinSeed=myWinSeed*214013L+2531011L)>>16) & 0x7fff );
}

bool getSystemRoamingDir(char *pDir, int iLen)
{
	if (NULL == pDir)
	{
		return FALSE;
	}

	memset(pDir, 0, iLen);

	//获取Roaming文件夹的绝对路径
	IMalloc * pShellMalloc = NULL;        // A pointer to the shell's IMalloc interface
	IShellFolder *psfParent;              // A pointer to the parent folder object's IShellFolder interface.
	LPITEMIDLIST pidlItem = NULL;         // The item's PIDL.
	LPITEMIDLIST pidlRelative = NULL;     // The item's PIDL relative to the parent folder.
	STRRET str;                           // The structure for strings returned from IShellFolder.
	CHAR szDisplayName[MAX_PATH]= {0,};   // The display name's string for Favorites

	HRESULT hres = SHGetMalloc( &pShellMalloc );
	if ( FAILED(hres) )
		return S_OK;
	hres = SHGetSpecialFolderLocation( NULL, CSIDL_APPDATA, &pidlItem );
	if ( FAILED(hres) )
		return S_OK;
	hres = SHBindToParent( pidlItem, IID_IShellFolder, (void**)&psfParent, (LPCITEMIDLIST*)&pidlRelative );
	if ( FAILED(hres) )
		return S_OK;
	// Retrieve the display name
	memset( &str, 0, sizeof(str) );
	hres = psfParent->GetDisplayNameOf( pidlRelative, SHGDN_NORMAL, &str );
	if ( FAILED(hres) )
		return S_OK;
	StrRetToBuf( &str, pidlItem, szDisplayName, ARRAYSIZE(szDisplayName) );
	memset( &str, 0, sizeof(str) );
	hres = psfParent->GetDisplayNameOf( pidlRelative, SHGDN_NORMAL | SHGDN_FORPARSING, &str );
	if ( FAILED(hres) )
		return S_OK;
	StrRetToBuf(&str, pidlItem, pDir, iLen);

	psfParent->Release();
	if ( pidlItem )
		pShellMalloc->Free( pidlItem );
	pShellMalloc->Release();

	return TRUE;
}
