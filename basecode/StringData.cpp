#include <cstdio>
#include <ctype.h>
#include <algorithm>

#include "basecode/StringData.h"
#include "basecode/basefunc.h"
#include "windows.h"

StringData::StringData()
{}

StringData::StringData( const char* str, int length ) : buf( str, length )
{

}

StringData::StringData( const char* str )
{
	if ( str != NULL )
		buf = str;
}


StringData StringData::ToString( const char *fmt, ... )
{
	char buff[1024];
#ifdef __ANDROID__
	va_list va_alist;
	va_start(va_alist, fmt);
	vsprintf(buff, fmt, va_alist);
	va_end(va_alist);
#else
	vsprintf( buff, fmt, (char*)((&fmt)+1) );
#endif
	StringData data = buff;
	return data;
}

StringData::StringData( const std::string& str)
{
    buf = str;
}


StringData::StringData( const StringData& data )
{
    buf = data.buf;
}

StringData&
StringData::operator=( const char* str )
{
    buf = str;
    return (*this);
}

StringData&
StringData::operator=( const StringData& data )
{
    if ( this != &data )
    {
        buf = data.buf;
    }

    return (*this);
}

const char*
StringData::c_str() const
{
    return buf.c_str();
}


StringData&
StringData::operator+=( char ch )
{
	buf += ch;
	return *this;
}


StringData StringData::substr( int start, int length ) const
{
	return StringData( buf.substr( start, length ) );
}


char
StringData::getChar( unsigned int i ) const
{
    if ( i > buf.size() )
        return 0;
    return buf[i];
}

void StringData::setchar( unsigned int i, char c )
{
    if ( i > buf.size() )
		return;
    else
        buf[i] = c;
}

char
StringData::operator[]( unsigned int i ) const
{
	if ( i > buf.size() )
		return 0;
	return buf[i];
}

uint32
StringData::length() const
{
    return (uint32)buf.size();
}

bool StringData::operator==( const char* str) const
{
    return ( buf == str );
}

bool StringData::operator==( const StringData& data ) const
{
    return ( buf == data.buf );
}

bool StringData::operator!=( const char* str ) const
{
    return ( buf != str);
}

bool StringData::operator!=( const StringData& data ) const
{
    return ( buf != data.buf );
}

void getDataSection( const StringData& data, unsigned int& start, StringData& dtRet, bool& bIsNumric )
{
	bIsNumric = ( data[start] <= '9' && data[start]>='0' );
	while( start < data.length() )
	{
		if ( bIsNumric )
		{
			if ( !(data[start] <= '9' && data[start]>='0') )
				break;
		}
		else
		{
			if ( data[start] <= '9' && data[start]>='0' )
				break;
		}
		dtRet += data[start++];
	}
}

bool
StringData::operator>( const StringData& data ) const
{
	return buf > data.buf;
	/*
	if ( length()>0 && data.length()==0 )
		return true;
	if ( length()==0 && data.length()>0 )
		return false;
	if ( length()==0 && data.length()==0 )
		return false;

	unsigned int pos1 = 0;
	unsigned int pos2 = 0;
	while ( true )
	{
		if ( pos1==length() && pos2==data.buf.length() )
			return false;
		if ( pos1==length() && pos2<data.buf.length() )
			return false;
		if ( pos2==length() && pos1<length() )
			return true;

		Data dtData1,dtData2;
		bool isNumeric1, isNumeric2;
		getDataSection( *this, pos1, dtData1, isNumeric1 );
		getDataSection( data, pos2, dtData2, isNumeric2 );
		if ( isNumeric1 && !isNumeric2 )
			return false;
		else if ( !isNumeric1 && isNumeric2 )
			return true;
		else if ( isNumeric1 && isNumeric2 )
		{
			if ( dtData1.length() > dtData2.length() )
			{
				return true;
			}
			else if ( dtData2.length() > dtData1.length() )
			{
				return false;
			}
			else
			{
				if ( dtData1.buf == dtData2.buf )
					continue;
				else
					return (dtData1.buf>dtData2.buf);
			}
		}
		else if ( !isNumeric1 && !isNumeric2 )
		{
			unsigned int len = min( dtData1.length(), dtData2.length() );
			for( unsigned int i=0; i<len; i++ )
			{
				char ch1 = dtData1[i];
				char ch2 = dtData2[i];
				bool bUpcase1,bUpcase2;
				if ( ch1>='A' && ch1<='Z' )
				{
					bUpcase1 = true;
					ch1 += ('a'-'A');
				}
				else
				{
					bUpcase1 = false;
				}
				if ( ch2>='A' && ch2<='Z' )
				{
					bUpcase2 = true;
					ch2 += ('a'-'A');
				}
				else
				{
					bUpcase2 = false;
				}
				if ( ch2 == ch1 )
				{
					if ( bUpcase1 == bUpcase2 )
						continue;
					else if ( bUpcase1 )
						return true;
					else
						return false;
				}
				else
				{
					return ch1>ch2;
				}
			}
			if ( dtData1.length() == dtData2.length() )
				continue;
			else if ( dtData1.length() > dtData2.length() )
				return true;
			else
				return false;
		}
	}
	*/
}

bool
StringData::operator<( const StringData& data ) const
{	
	return buf < data.buf;
	/*
	if ( *this>data || *this==data )
		return  false;
	else
		return true;
		*/
}

StringData
StringData::operator+( const StringData& data ) const
{
    return ( buf + data.buf );
}



StringData
StringData::operator+( const char* str ) const
{
    return ( buf + str );
}

StringData&
StringData::operator+=( const StringData& data )
{
	buf += data.buf;
	return *this;
}


StringData&
StringData::operator+=( const char* str )
{
	buf += str;
	return *this;
}

StringData&
StringData::operator+=( int val )
{
	char buff[10];
	sprintf( buff, "%d", val );
	buf += buff;
	return *this;
}

StringData&
StringData::operator+=( unsigned int val )
{
	char buff[10];
	sprintf( buff, "%u", val );
	buf += buff;
	return *this;
}

StringData::~StringData()
{}

void
StringData::clear()
{
    buf.erase();
}


int32
StringData::toInt32() const
{
    return atoi( buf.c_str() );
}


int64
StringData::toInt64() const
{
	return _atoi64( buf.c_str() );
}



float
StringData::toFloat() const
{
	return (float)atof(buf.c_str());
}


int StringData::HexToInt()
{
	int val = 0;
	int j=0;
	std::string buff;
	bool bNega = false;
	if ( buf[0] == '-' )
	{
		buff = buf.substr( 1 );
		bNega = true;
	}
	else
	{	
		buff = buf;
	}
	for ( int i=(int)buff.length()-1; i>=0; i--,j++ )
	{
		char ch = buff[i];
		if ( ch>='a' && ch <='f')
		{
			val += ( (ch-'a'+0xa) << (j<<2) );
		}
		else if ( ch>='A' && ch <='F' )
		{
			val += ( (ch-'A'+0xa) << (j<<2) );
		}
		else if ( ch>='0' && ch <='9' )
		{
			val += ( (ch-'0') << (j<<2) );
		}
		else
		{
			return 0;
		}
	}
	if ( bNega )
		return -val;
	else
		return val;
}

int32 StringData::find( const StringData& match, int start ) const
{
    return (int32)buf.find( match.buf, start );
}

int
StringData::findNoCase( const StringData& match, int start/*=0*/ )
{
	const char* pStrCompare = match.c_str();
	const char* pStr = this->c_str();
	int iLen = match.length();
	int iMaxLen = length();
	for ( int i=0; i<=iMaxLen-iLen; i++ )
	{
		if ( !compareNoCase( pStr+i, pStrCompare, iLen ) )
			return i;
	}
	return -1;
}

void
StringData::removeQuo()
{
	uint32 first=0,end=buf.length();
	uint32 i = 0;
	for ( i=0; i<(uint32)buf.length()-1; i++ )
	{
		if( buf[i] == '"')
			first = i+1;
		else
			break;
	}
	for ( i=(int)buf.length(); i>first; i-- )
	{
		if ( buf[i] == '"')
			end = i;
		else
			break;
	}
	buf = buf.substr( first, end-first );
}


bool StringData::match( const StringData& match, StringData* retModifiedData, bool replace, StringData replaceWith )
{
    int32 pos = (int32)buf.find( match.buf );
    if ( pos == -1 )
    {
        return false;
    }
    unsigned int replacePos = pos + match.length();
    if ( retModifiedData )
    {
        (*retModifiedData) = buf.substr( 0, pos );
    }
    if ( replace )
    {
        if ( replacePos <= buf.size() )
        {
            buf.replace( 0, replacePos, replaceWith.buf );
        }
    }
    return true;
}


void
StringData::replace( int startpos, int len, const StringData& replaceStr )
{
	buf.replace( startpos, len, replaceStr.buf );
}

void StringData::replace( const StringData& from, const StringData to )
{
	int pos = 0;
	while ( (pos=find(from)) != -1 )
	{
		replace( pos, from.length(), to );
	}
}


int
StringData::findlast( const StringData& match, int stop ) const
{
	return (int)buf.find_last_of( match.buf, stop );
}


int
StringData::findlastNoCase( const StringData& match, int stop/*=-1*/ ) const
{
	const char* pStr = buf.c_str();
	const char* pCompare = match.c_str();
	uint32 len = match.length();
	for( int i=stop-len+1; i>=0; i-- )
	{
		if ( !compareNoCase( pStr+i, pCompare, len ) )
			return i;
	}
	return -1;
}


bool isEqualNoCase( const StringData& left, const StringData& right )
{
	if ( left.length() != right.length() )
		return false;
	for ( unsigned int i=0; i<left.length(); i++ )
    {
        if ( toupper(left[i]) != toupper(right[i]) )
            return false;
    }
    return true;
}

bool StringData::isEqualNoCase( const StringData& data ) const
{
	if ( length() != data.length() )
		return false;
	for ( unsigned int i=0; i<data.length(); i++ )
	{
		if ( toupper(buf[i]) != toupper(data[i]) )
			return false;
	}
	return true;
}

void StringData::removeChar( char ch )
{
	int len = (int)buf.length();
	if ( len == 0 )
		return;
	int begin = 0;
	for ( ; begin<len; begin++ )
	{
		if ( buf[begin] != ch )
			break;
	}
	int end = len-1;
	for ( ; end>=0; end-- )
	{
		if ( buf[end] != ch )
			break;
	}
	if ( begin==0 && end==len-1 )
		return;
	buf = buf.substr( begin, end+1-begin );
}

void StringData::removeSpaces()
{
	removeChar( ' ' );
}

void StringData::clearSpaces()
{
    if ( buf.length() == 0 )
    {
        return ;
    }	
    int32 beforeval;
    do
    {
        beforeval = (int32)buf.find( SPACE );
        if( beforeval == -1 )
        {
            break;
        }
        buf.replace( beforeval, strlen( SPACE ) , "" );
    }
    while( beforeval == 0 );
}

void StringData::clearAllSpaces()
{
	if ( buf.length() == 0 )
	{
		return ;
	}	
	int32 beforeval;
	do
	{
		beforeval = (int32)buf.find( SPACE );
		if( beforeval == -1 )
		{
			break;
		}
		buf.replace( beforeval, strlen( SPACE ) , "" );
	}
	while( true );
}

void
StringData::expand( StringData startFrom, StringData findstr, StringData replstr, StringData delimiter )
{
    int32 startPos = (int32)buf.find( startFrom.c_str() );
    if ( startPos < -1 )
    {
        int32 delimPos = (int32)buf.find( delimiter.c_str(), startPos );
        int32 findPos = (int32)buf.find( findstr.c_str(), startPos );
        while ( findPos < delimPos )
        {
            //found replstr, replace
            buf.replace( findPos, strlen(findstr.c_str()), replstr.c_str());
            //find next.
            //delimPos = buf.find( delimiter.getData(), findPos);
			delimPos = (int32)buf.find( delimiter.c_str(), findPos+static_cast<std::string>(replstr.c_str()).size() );
            findPos = (int32)buf.find( findstr.c_str(), findPos );
        }
    }
}


bool
StringData::isEmpty() const
{
	return buf.empty();
}

void
StringData::removeCRLF()
{
	removeChar( '\r' );
	removeChar( '\n' );
}

void
StringData::removeTab()
{
	removeChar( '\t' );
}

StringData
StringData::xorString( const StringData& str1, const StringData& str2 )
{
	int maxLen = str1.length() > str2.length() ? str1.length() : str2.length();
	int minLen = str1.length() > str2.length() ? str2.length() : str1.length();
	char *buff = new char[maxLen];
	int i = 0;
	for( i=0; i<minLen; i++ )
	{
		buff[i] = str1[i] ^ str2[i];
	}

	if ( str1.length() > str2.length() )
	{
		for( i=minLen; i<maxLen; i++ )
			buff[i] = ~str1[i];
	}
	else
	{
		for( i=minLen; i<maxLen; i++ )
			buff[i] = ~str2[i];
	}
	StringData data = StringData( buff, maxLen );
	delete[] buff;
	return data;
}

void
StringData::split( const StringData& sp, std::vector<StringData>& outElements ) const
{
	StringData dtVal;
	StringData data( *this );
	//int i = 0;
	while( data.match( sp, &dtVal, true ) )
	{
		outElements.push_back( dtVal );
	}
	if ( data.length() > 0 )
		outElements.push_back( data );
}


void
StringData::inflate( int size, const StringData &fillval, bool before )
{
	if ( (int)buf.length() >= size ) 
		return;
	StringData dtVal;
	int iInflateSize = size - (uint32)buf.length();
	for( unsigned int i=0; i<iInflateSize/fillval.length(); i++ )
	{
		dtVal += fillval;
	}
	if ( iInflateSize%fillval.length() > 0 )
		dtVal += fillval.substr( 0, iInflateSize%fillval.length() );
	if ( before )
		operator =( dtVal + *this );
	else
		operator =( *this + dtVal );
}

/*
bool StringData::tobcd( char fillch )
{
	int len;
	if ( buf.size() % 2 > 0 )
		len = buf.size() + 1;
	else
		len = buf.size();
	int bcdLen = len>>1;
	char* buff = new char[ len ];
	char* buff1 = new char[ bcdLen ];
	memcpy( buff, buf.c_str(), buf.size() );
	if ( len != (int)buf.size() )
		buff[ len-1 ] = fillch;
	bool bRet;
	if ( bRet = ascBcd( buff, (unsigned char*)buff1, bcdLen ) )
	{
		buf.assign( buff1, bcdLen );
	}
	delete[] buff;	delete[] buff1;
	return bRet;
}
*/


bool StringData::isNumeric()
{
	if ( buf.size() == 0 )
		return false;
	for ( unsigned int i=0; i<buf.size(); i++ )
	{
		if ( buf[i]>'9' || buf[i]<'0' )
			return false;
	}
	return true;
}


bool StringData::isEndWith( const StringData& str, bool bCase ) const
{
	if ( buf.length() < str.length() )
		return false;
	StringData subStr = buf.substr( buf.length()-str.length() );
	if ( bCase )
	{
		if ( subStr == str )
			return true;
		else
			return false;
	}
	else
	{
		if ( compareNoCase(subStr.c_str(), str.c_str(), str.length()) == 0 )
			return true;
		else
			return false;
	}
}


StringData StringData::getFilePath() const
{
	int32 index1 = (int32)buf.find_last_of( "\\" );
	int32 index2 = (int32)buf.find_last_of( "/" );
	int32 index = __max( index1, index2 );
	if ( index == -1 )
	{
		return "";
	}
	else if ( index==((int)buf.length()-1) )
	{
		return *this;
	}
	else
	{
		StringData ret = substr( 0,index+1 );
		return ret;
	}
}


void StringData::makePath()
{
	if ( buf.length() == 0 )
		return;
	char ch = buf[buf.length()-1];
	if ( ch!='/' && ch!='\\' )
		buf += '/';
}


StringData StringData::getFileName() const
{
	int32 index = (int32)buf.find_last_of( "\\" );
	if ( index == -1 )
		index = (int32)buf.find_last_of( "/" );
	if ( index==-1 )
		return *this;
	if ( index==((int)buf.length()-1) )
		return "";
	return substr( index+1 );
}


StringData StringData::getExtName() const
{
	StringData strExt;
	int index = findlast( "." );
	if ( index != -1 )
		strExt = substr( index+1 );
	return strExt;
}

void StringData::lowerCase()
{
	uint32 size = (uint32)buf.size();
	for ( unsigned int i=0; i<size; i++ )
		buf[i] = tolower( buf[i] );
}


void StringData::ToLower()
{
	transform( buf.begin(), buf.end(), buf.begin(), tolower );
}

void StringData::formatPath( bool url )
{
	uint32 len = buf.length();
	if ( url )
	{
		for ( uint32 i=0; i<len; i++ )
		{
			if ( buf[i] == '\\' )
				buf[i] = '/';
		}
		std::string::size_type pos;
		while ( (pos=find("//")) != std::string::npos )
			replace( pos, 2, "/" );
	}
	else
	{
		for ( uint32 i=0; i<len; i++ )
		{
			if ( buf[i] == '/' )
				buf[i] = '\\';
		}
		std::string::size_type pos;
		while ( (pos=find("\\\\")) != std::string::npos )
			replace( pos, 2, "\\" );
	}
}
