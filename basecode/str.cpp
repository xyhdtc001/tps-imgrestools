#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "basecode/BaseFunc.h"
#include "basecode/str.h"
#include "basecode/mempool.h"
#include "basecode/platformIntrinsics.h"
#include "Windows.h"

#include "basecode/unicode.h"

//Sigh... guess what compiler needs this...
namespace DictHash 
{ 
	uint32 hash( String::StringData* ); 
}

namespace KeyCmp
{
   template< typename Key > bool equals( const Key&, const Key& );
   template<> bool equals<>( String::StringData* const&, String::StringData* const& );
}

#ifndef WIN32
#ifndef METRO
char* strlwr(char* str)
{
    char* orig = str;
    for ( ; *str != '\0'; str++) {
        *str = tolower(*str);
    }
    return orig;
}

char* strupr(char* str)
{
    char* orig = str;
    for ( ; *str != '\0'; str++) {
        *str = toupper(*str);
    }
    return orig;
}
#endif
#endif

/*
#include "basecode/string/dictionary.h"
#include "basecode/string/stringFunctions.h"
#include "basecode/string/hashFunction.h"
#include "basecode/string/unicode.h"
#include "basecode/string/autoPtr.h"
#include "basecode/string/dataChunker.h"
#include "basecode/string/platformIntrinsics.h"
*/


const String::SizeType String::NPos = uint32(~0);
const String String::EmptyString;

/// A delete policy for the AutoPtr class
struct DeleteString
{
	template<class T>
	static void destroy(T *ptr) { free(ptr); }
};


//-----------------------------------------------------------------------------
/// Search for a character.
/// Search for the position of the needle in the haystack.
/// Default mode is StrCase | StrLeft, mode also accepts StrNoCase and StrRight.
/// If pos is non-zero, then in mode StrLeft the search starts at (hay + pos) and
/// in mode StrRight the search starts at (hay + pos - 1)
/// @return Returns a pointer to the location of the character in the haystack or 0
static const char* StrFind( const char* hay, char needle, int32 pos, uint32 mode )
{
   if ( mode & String::Right )
   {
      // Go to the end first, then search backwards
      const char  *he = hay;

      if (pos)
      {
         he += pos - 1;
      }
      else
      {
         while (*he)
            he++;
      }

      if ( mode & String::NoCase )
      {
         needle = tolower( needle );

         for ( ; he >= hay; he-- )
         {
            if (tolower(*he) == needle)
               return he;
         }
      }
      else
      {
         for (; he >= hay; he--)
         {
            if (*he == needle)
               return he;
         }
      }
      return 0;
   }
   else
   {
      if (mode & String::NoCase)
      {
         needle = tolower(needle);
         for (hay += pos; *hay && tolower(*hay) != needle;)
            hay++;
      }
      else
      {
         for (hay += pos; *hay && *hay != needle;)
            hay++;
      }

      return *hay ? hay : 0;
   }
}

/// Search for a StringData.
/// Search for the position of the needle in the haystack.
/// Default mode is StrCase | StrLeft, mode also accepts StrNoCase and StrRight.
/// If pos is non-zero, then in mode StrLeft the search starts at (hay + pos) and
/// in mode StrRight the search starts at (hay + pos - 1)
/// @return Returns a pointer to the StringData in the haystack or 0
static const char* StrFind( const char* hay, const char* needle, int32 pos, uint32 mode )
{
   if (mode & String::Right)
   {
      const char *he = hay;
      if ( pos )
      {
         he += pos - 1;
      }
      else
      {
         while (*he)
            he++;
      }

      if ( mode & String::NoCase )
      {
         //AutoPtr<char,DeleteString> ln(dStrlwr(dStrdup(needle)));
		 char ch = *needle;
         for ( ; he >= hay; he-- )
         {
            if ( tolower(*he) == ch )
            {
               uint32 i = 1;
               while (needle[i] && tolower(needle[i])==tolower(he[i]) )
                  i++;
               if ( !needle[i] )
                  return he;
            }
         }
      }
      else
      {
         for (; he >= hay; he--)
         {
            if (*he == *needle)
            {
               uint32 i = 0;
               while (needle[i] && needle[i] == he[i])
                  i++;
               if (!needle[i])
                  return he;    
            }
		}
      }  
      return 0;
   }
   else
   {
      if ( mode & String::NoCase )
      {
         //AutoPtr<char,DeleteString> ln(dStrlwr(dStrdup(needle)));
		 char ch = tolower( *needle );
         for ( hay += pos; *hay; hay++ )
         {
            if ( tolower(*hay) == ch )
            {
               uint32 i = 1;
               while ( needle[i] && tolower(needle[i])==tolower(hay[i]) )
                  i++;
               if ( !needle[i] )
                  return hay;
            }
         }
      }
      else
      {
         for ( hay += pos; *hay; hay++ )
         {
            if ( *hay == *needle )
            {
               uint32 i = 1;
               while ( needle[i] && needle[i] == hay[i] )
                  i++;
               if (!needle[i])
                  return hay;
            }
         }
      }
   }
   return 0;
}

//uint32 dStrlen(const UTF16 *unistring)
//{
//	if(!unistring)
//		return 0;
//
//	uint32 i = 0;
//	while(unistring[i] != '\0')
//		i++;
//
//	return i;
//}
//
//uint32 dStrlen(const UTF8 *str)
//{
//	return (uint32)strlen(str);
//}   

inline int  dStricmp(const UTF8 *str1, const UTF8 *str2)
{
#ifdef WINDOWS
	return _stricmp( str1, str2 );
#else
    return strncmp( str1, str2, strlen(str1));
#endif
}

inline int  dStrcmp(const UTF8 *str1, const UTF8 *str2)
{
	return strcmp( str1, str2 );
}



/*
UTF16* MByteToWChar(const UTF8* lpcszStr)
{	
#ifdef WINDOWS
	//uint32 len = dStrlen(lpcszStr) + 1;
	//static UTF16 buf[2048] = {0};

	uint32 nCodepoints = MultiByteToWideChar (936, 0, lpcszStr, -1, NULL, 0);

	UTF16 *ret = new UTF16[nCodepoints];
	MultiByteToWideChar (936, 0, lpcszStr, -1, (LPWSTR)(ret), nCodepoints);  
	//memcpy(ret, buf, nCodepoints * sizeof(UTF16));
	return ret;
#else
    return convertUTF8toUTF16(ConvertEnc("GBK", "UTF-8", lpcszStr));
#endif
}


UTF8* WCharToMByte(const UTF16* lpcwszStr)
{
#ifdef WINDOWS
	//uint32 len = dStrlen(lpcwszStr) * 3 + 1;
	//static UTF8 buf[2048] = {0};

	uint32 nCodeunits = WideCharToMultiByte(936,NULL, (LPWSTR)lpcwszStr, -1, NULL, 0, NULL, FALSE);

	UTF8 *ret = new UTF8[nCodeunits];
	WideCharToMultiByte(936,NULL,(LPWSTR)lpcwszStr, -1, ret, nCodeunits,NULL,FALSE);
	//memcpy(ret, buf, nCodeunits * sizeof(UTF8));
	return ret;
#else
    return ConvertEnc("UTF-8", "GBK",  convertUTF16toUTF8( lpcwszStr));
#endif
}*/



//-----------------------------------------------------------------------------

/// Struct with String::StringData's field so we can initialize
/// this without a constructor.

#pragma pack(1)
struct StringDataImpl
{
      StringChar*       mString;       ///< so we can inspect data in a debugger

      uint32			mRefCount;     ///< String reference count; string is not refcounted if this is U32_MAX (necessary for thread-safety of interned strings and the empty string).
      uint32			mLength;       ///< String length in bytes excluding null.
      mutable uint32	mNumChars;     ///< Character count; varies from byte count for strings with multi-bytes characters.
      //mutable uint32	mHashCase;     ///< case-sensitive hash
      //mutable uint32	mHashNoCase;   ///< case-insensitive hash
      mutable UTF16*    mUTF16;
      StringChar        mData[1];      ///< Start of string data
};
#pragma pack()

///
class String::StringData : protected StringDataImpl
{
public:

	void* operator new( size_t size, size_t len )
	{
		/*
		if (_memPool == NULL)
		_memPool = new MemPool< 64, 1000 >;
		StringData *str = reinterpret_cast<StringData*>(_memPool->alloc( size + len));
		str->mLength = len;
		*/
		StringData *str = (StringData *)malloc( size+len );
		str->mLength = (uint32)len;
		return str;
	}

	void operator delete( void * p, size_t size )
	{
		/*
		if (_memPool == NULL)
		return;
		_memPool->dealloc( p );
		*/
		free( p );
	}
	
	void operator delete( void * p)
	{
		/*
		if (_memPool == NULL)
		return;
		_memPool->dealloc( p );
		*/
		free( p );
	}

	StringData( const StringChar* data )
	{
		mRefCount = 1;
		mNumChars = uint32(0xffffffff);
		//mHashNoCase = uint32(0xffffffff);
		mUTF16 = NULL;
		if ( data )
		{
			memcpy( mData, data, sizeof( StringChar ) * mLength );
			mData[ mLength ] = '\0';
		}
		mString = &mData[0];
	}

	StringData( const UTF16* data16 )
	{
		mRefCount = 1;
		mNumChars = uint32(0xffffffff);
		//mHashNoCase = uint32(0xffffffff);
		mUTF16 = NULL;  
		StringChar* data = WCharToMByte( data16 );
		if ( data )
		{
			memcpy( mData, data, sizeof( StringChar ) * mLength );
			mData[ mLength ] = '\0';
			delete data;
		}
		mString = &mData[0];
	}

	~StringData()
	{		
		if( mUTF16)
		{
			delete[] mUTF16;
			mUTF16 = NULL;
		}
	}


      bool isShared() const
      {
         return ( mRefCount > 1 );
      }

      void addRef()
      {
			mRefCount ++;
      }

      void release()
      {
            -- mRefCount;
            if( !mRefCount )
               delete this;
      }

      uint32 getLength() const
      {
         return mLength;
      }

      uint32 getDataSize() const
      {
         return ( mLength + 1 );
      }

      uint32 getDataSizeUTF16() const
      {
         return ( mLength * sizeof( UTF16 ) );
      }

      UTF8 operator []( uint32 index ) const
      {
         return mData[ index ];
      }

      UTF8* utf8()
      {
         return mData;
      }

      const UTF8* utf8() const
      {
         return mData;
      }

	  UTF16* utf16() const
	  {
		  if( !mUTF16 )
		  {
			  UTF16* utf16 = MByteToWChar( mData );
// 			  if( !dCompareAndSwap( mUTF16,( UTF16* ) NULL, utf16 ) )
// 				  delete [] utf16;

			  mUTF16 = utf16;
		  }
		  return mUTF16;
	  }

	  /*
      uint32 getOrCreateHashCase() const
      {
         if( mHashCase == uint32(0xffffffff) )
         {
            mHashCase = fastHash( (mData), mLength, 0 );
         }
         return mHashCase;
      }

      uint32 getHashNoCase() const
      {
         return mHashNoCase;
      }

      uint32 getOrCreateHashNoCase() const
      {
         if( mHashNoCase == uint32(0xffffffff))
         {
            UTF8 *lower = new UTF8[ mLength + 1 ];
            strncpy( lower, utf8(), mLength );
            lower[ mLength ] = 0;
            strlwr( lower );
            mHashNoCase = fastHash( lower, mLength, 0 );
            delete [] lower;
         }

         return mHashNoCase;
      }
	  */

      uint32 getNumChars() const
      {
		  if( mNumChars == uint32(0xffffffff) )
			  mNumChars = dStrlen( utf16() );

		  return mNumChars;
      }

	  UTF16 getChar(SizeType index)
	  {
		  if( mNumChars == uint32(0xffffffff) )
			  mNumChars = dStrlen( utf16() );

		  return utf16()[index];
	  }

      static StringData* Empty()
      {
         static UTF16 emptyUTF16[ 1 ] = { 0 };
         static StringDataImpl empty =
         {       
            "",            // mString            
            1,			   // mRefCount
            0,             // mLength
            0,             // mNumChars
            //0,             // mHashCase
            //0,             // mHashNoCase
            emptyUTF16,    // mUTF16
            { 0 }          // mData
         };
         empty.mRefCount++;
         return ( StringData* ) &empty;
      }
};

//MemPool< 64, 1000 >* String::StringData::_memPool = NULL;
//-----------------------------------------------------------------------------

/*
namespace DictHash
{
	inline uint32 hash( String::StringData* data )
	{
		return data->getOrCreateHashCase();
	}
}
*/

namespace KeyCmp
{
	template<>
	inline bool equals<>( String::StringData* const& d1, String::StringData* const& d2 )
	{
		return ( strcmp( d1->utf8(), d2->utf8() ) == 0 );
	}
}


String::String()
{
   _string = StringData::Empty();
}

String::String(const String &str)
{
   _string = str._string;
   _string->addRef();
}

String::String(const StringChar *str)
{
	if ( str && *str )
	{
		uint32 len = (uint32)strlen(str);
		_string = new ( len ) StringData( str );
	}
	else
		_string = StringData::Empty();
}

String::String(const StringChar *str, SizeType len)
{
	if ( str && *str && len!=0 )
		_string = new ( len ) StringData( str );
	else
		_string = StringData::Empty();
}

String::String(const UTF16 *str)
{
	if( str && str[ 0 ] )
	{
		UTF8* utf8 = WCharToMByte( str );
		uint32 len = dStrlen( utf8 );
		_string = new ( len ) StringData( utf8 );
		delete [] utf8;
	}
	else
		_string = StringData::Empty();
}

String::String(SizeType num, const StringChar c)
{
	if( !num )
		_string = StringData::Empty();
	else
	{
		_string = new ( num+1 ) StringData( (StringChar*)NULL );

		UTF8* p = _string->utf8();
		while(num--)
			*p++ = c;
		*p = 0;
	}
}

String::~String()
{
   _string->release();
}

//-----------------------------------------------------------------------------

const StringChar* String::c_str() const
{
   return _string->utf8();
}

const UTF16 *String::utf16() const
{
	return _string->utf16();
}

int32 String::toInt32()
{
	return atoi( _string->utf8() );
}

ulong String::toULong(int base)
{
	return strtoul(_string->utf8(), NULL, base);
}

int64 String::toInt64()
{
	return _atoi64( _string->utf8() );
}

float32 String::toFloat32()
{
	return (float32)atof( _string->utf8() );
}


String::SizeType String::length() const
{
   return _string->getLength();
}

String::SizeType String::size() const
{
   return _string->getDataSize();
}

String::SizeType String::numChars() const
{
   return _string->getNumChars();
}

bool String::isEmpty() const
{
   return ( _string == StringData::Empty() );
}

bool String::isShared() const
{
   return _string->isShared();
}

bool String::isSame( const String& str ) const
{
   return ( _string == str._string );
}

/*
uint32 String::getHashCaseSensitive() const
{
   return _string->getOrCreateHashCase();
}


uint32 String::getHashCaseInsensitive() const
{
   return _string->getOrCreateHashNoCase();
}
*/


String::SizeType String::find(const String &str, SizeType pos, uint32 mode) const
{
   return find(str._string->utf8(), pos, mode);
}

String& String::insert(SizeType pos, const String &str)
{
   return insert(pos, str._string->utf8());
}

String& String::replace(SizeType pos, SizeType len, const String &str)
{
   return replace(pos, len, str._string->utf8());
}

//-----------------------------------------------------------------------------

String& String::operator=(StringChar c)
{
   _string->release();

   _string = new ( 2 ) StringData( (StringChar*)NULL );
   _string->utf8()[ 0 ] = c;
   _string->utf8()[ 1 ] = '\0';

   return *this;
}

String& String::operator+=( StringChar c )
{
   // Append the given string into a new string
   uint32 len = _string->getLength();
   StringData* sub = new ( len + 1 ) StringData( (StringChar*)NULL );

   copy( sub->utf8(), _string->utf8(), len );
   sub->utf8()[len] = c;
   sub->utf8()[len+1] = 0;

   _string->release();
   _string = sub;

   return *this;
}

//-----------------------------------------------------------------------------

String& String::operator=(const StringChar *str)
{
   // Protect against self assignment which is not only a
   // waste of time, but can also lead to the string being
   // freed before it can be reassigned.
   if ( _string->utf8() == str )
      return *this;

   _string->release();

   if (str && *str)
   {
      uint32 len = (uint32)strlen(str);
      _string = new ( len ) StringData( str );
   }
   else
      _string = StringData::Empty();

   return *this;
}

String& String::operator=(const String &src)
{
   // Inc src first to avoid assignment to self problems.
   src._string->addRef();

   _string->release();
   _string = src._string;

   return *this;
}

String& String::operator+=(const StringChar *src)
{
   if( src == NULL && !*src )
      return *this;

   // Append the given string into a new string
   uint32 lena = _string->getLength();
   uint32 lenb = (uint32)strlen(src);
   uint32 newlen = lena + lenb;

   StringData* sub;
   if( !newlen )
      sub = StringData::Empty();
   else
   {
      sub = new ( newlen ) StringData( (StringChar*)NULL );

      copy(sub->utf8(),_string->utf8(),lena);
      copy(sub->utf8() + lena,src,lenb + 1);
   }

   _string->release();
   _string = sub;

   return *this;
}

String& String::operator+=(const String &src)
{
   if( src.isEmpty() )
      return *this;

   // Append the given string into a new string
   uint32 lena = _string->getLength();
   uint32 lenb = src._string->getLength();
   uint32 newlen = lena + lenb;

   StringData* sub;
   if( !newlen )
      sub = StringData::Empty();
   else
   {
      sub = new ( newlen ) StringData( (StringChar*)NULL );

      copy(sub->utf8(),_string->utf8(),lena);
      copy(sub->utf8() + lena,src._string->utf8(),lenb + 1);
   }

   _string->release();
   _string = sub;

   return *this;
}

//-----------------------------------------------------------------------------

String operator+(const String &a, const String &b)
{  
   if( a.isEmpty() )
      return b;
   else if( b.isEmpty() )
      return a;

   uint32 lena = a.length();
   uint32 lenb = b.length();

   String::StringData *sub = new ( lena + lenb ) String::StringData( (StringChar*)NULL );

   String::copy(sub->utf8(),a._string->utf8(),lena);
   String::copy(sub->utf8() + lena,b._string->utf8(),lenb + 1);

   return String(sub);
}

String operator+(const String &a, StringChar c)
{
   uint32 lena = a.length();
   String::StringData *sub = new ( lena + 1 ) String::StringData( (StringChar*)NULL );

   String::copy(sub->utf8(),a._string->utf8(),lena);

   sub->utf8()[lena] = c;
   sub->utf8()[lena+1] = 0;

   return String(sub);
}

String operator+(StringChar c, const String &a)
{
   uint32 lena = a.length();
   String::StringData *sub = new ( lena + 1 ) String::StringData( (StringChar*)NULL );

   String::copy(sub->utf8() + 1,a._string->utf8(),lena + 1);
   sub->utf8()[0] = c;

   return String(sub);
}

String operator+(const String &a, const StringChar *b)
{
   if( a.isEmpty() )
      return String( b );

   uint32 lena = a.length();
   uint32 lenb = (uint32)strlen(b);

   if( !lenb )
      return a;

   String::StringData *sub = new ( lena + lenb ) String::StringData( (StringChar*)NULL );

   String::copy(sub->utf8(),a._string->utf8(),lena);
   String::copy(sub->utf8() + lena,b,lenb + 1);

   return String(sub);
}

String operator+(const StringChar *a, const String &b)
{

   if( b.isEmpty() )
      return String( a );

   uint32 lena = (uint32)strlen( a );
   if( !lena )
      return b;

   uint32 lenb = b.length();

   String::StringData* sub = new ( lena + lenb ) String::StringData( (StringChar*)NULL );

   String::copy(sub->utf8(),a,lena);
   String::copy(sub->utf8() + lena,b._string->utf8(),lenb + 1);

   return String(sub);
}

bool String::operator==( const String &str ) const
{
   if ( str._string == _string )
      return true;
   else if( str.length() != length() )
      return false;
   else
      return ( memcmp( str._string->utf8(), _string->utf8(), _string->getLength() ) == 0 );
}

bool String::operator==( const StringChar* str ) const
{
	uint32 len = (uint32)strlen( str );
	if ( length() != len )
		return false;
	return ( memcmp(str,_string->utf8(),len) == 0 );
}

bool String::operator==( StringChar c ) const
{
   if( !_string || _string->getLength() != 1 )
      return false;
   else
      return ( _string->utf8()[ 0 ] == c );
}

bool String::operator<( const String &str ) const
{
   return ( strcmp(_string->utf8(),str._string->utf8()) < 0 );
}

bool String::operator>(const String &str) const
{
   return ( strcmp(_string->utf8(),str._string->utf8()) > 0 );
}

bool String::operator<=(const String &str) const
{
   return ( strcmp(_string->utf8(),str._string->utf8()) <= 0 );
}

bool String::operator>=(const String &str) const
{
   return ( strcmp(_string->utf8(),str._string->utf8()) >= 0 );
}


bool String::match( const String& match, String* retModifiedData, bool bReplace )
{
	int32 pos = find( match );
	if ( pos == NPos )
		return false;
	if ( retModifiedData != NULL )
		*retModifiedData = substr( 0, pos );
	if ( bReplace )
		*this = substr( pos + match.length() );
	return true;
}


void String::removeChar( char ch )
{
	int len = length();
	if ( len == 0 )
		return;
	int begin = 0;
	char* buf = _string->utf8();
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
	if (end+1-begin<begin)
	{
		*this = "";
		return;
	}
	*this = substr( begin, end+1-begin );
}

void String::removeSpaces()
{
	removeChar( ' ');
}

void String::removeCRLF()
{
	removeChar( '\r' );
	removeChar( '\n' );
}

void String::removeTab()
{
	removeChar( '\t' );
}

bool String::isEndWith( const String& str, uint32 mode ) const
{
	uint32 len1 = str.length();
	uint32 len2 = length();
	if ( len1 > len2 )
		return false;
	return substr(len2-len1).equal( str, mode );
}


int32 String::HexToInt()
{
	if ( _string->getLength() == 0 )
		return 0;
	int32 val = 0;
	uint32 j = 0;
	bool bNega = false;
	uint32 start = 0;
	uint32 len = _string->getLength();
	UTF8* buff = _string->utf8();
	if ( buff[0] == '-' )
	{
		bNega = true;
		start = 1;
	}
	for ( uint32 i=len-1; i>=start; i--,j++ )
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

//-----------------------------------------------------------------------------
// Base functions for string comparison

int32 String::compare(const StringChar *str, SizeType len, uint32 mode) const
{
   const StringChar  *p1 = _string->utf8();
   const StringChar  *p2 = str;

   if (p1 == p2)
      return 0;

   if( mode & String::Right )
   {
      uint32 n = len;
      if( n > length() )
         n = length();

      p1 += length() - n;
      p2 += strlen( str ) - n;
   }

   if (mode & String::NoCase)
   {
      if (len)
      {
         for (;--len; p1++,p2++)
         {
            if (tolower(*p1) != tolower(*p2) || !*p1)
               break;
         }
      }
      else
      {
         while (tolower(*p1) == tolower(*p2) && *p1)
         {
            p1++;
            p2++;
         }
      }

      return tolower(*p1) - tolower(*p2);
   }

   if (len)
      return memcmp(p1,p2,len);

   while (*p1 == *p2 && *p1)
   {
      p1++;
      p2++;
   }

   return *p1 - *p2;
}

int32 String::compare(const String &str, SizeType len, uint32 mode) const
{
   if ( str._string == _string )
      return 0;

   return compare( str.c_str(), len, mode );
}

bool String::equal( const String &str, uint32 mode ) const
{
   if ( !mode )
   {
      return ( *this == str );
   }
   else
   {
      if ( _string == str._string )
         return true;
      else if ( length() != str.length() )
         return false;
      else
         return ( compare( str.c_str(), length(), mode ) == 0 );
   }
}

//-----------------------------------------------------------------------------

String::SizeType String::find(StringChar c, SizeType pos, uint32 mode) const
{
   const StringChar* ptr = StrFind(_string->utf8(),c,pos,mode);
   return ptr? SizeType(ptr - _string->utf8()): NPos;
}

String::SizeType String::find(const StringChar *str, SizeType pos, uint32 mode)  const
{
   const StringChar* ptr = StrFind(_string->utf8(),str,pos,mode);
   return ptr? SizeType(ptr - _string->utf8()): NPos;
}

String::SizeType String::findFirst(const String &str, SizeType pos, uint32 mode) const
{
	SizeType i = 0;
	const SizeType len = length();
	while ( i < NPos && ( pos + i ) < len )
	{
		StringChar ch = c_str()[pos + i];
		if ( str.find( ch, 0, mode) != NPos)
			return pos + i;
		i += 1; 
	}
	return NPos;
}

String::SizeType String::findLast(const String &str, SizeType pos, uint32 mode) const
{
	SizeType i = 0;
	const SizeType len = length();
	if ( pos > len ) pos = len - 1;

	while ( i < NPos && ( pos - i ) != NPos )
	{
		SizeType j = pos - i;

		// and back to the usual dull test
		StringChar ch = c_str()[j];
		if ( str.find( ch, 0, mode) != NPos )
			return j;
		i++;
	}
	return NPos;
}

String::SizeType String::findFirstNot(const String &str, SizeType pos, uint32 mode) const
{
	SizeType i = 0;
	const SizeType len = length();
	while ( i < NPos && ( pos + i ) < len )
	{
		StringChar ch = c_str()[pos + i ];
		if ( str.find( ch, 0, mode) == NPos)
			return pos + i;
		i += 1 ; // increment by the Unicode character length
	}
	return NPos;
}

String::SizeType String::findLastNot(const String &str, SizeType pos, uint32 mode) const
{
	SizeType i = 0;
	const SizeType len = length();
	if ( pos > len ) pos = len - 1;

	while ( i < NPos && ( pos - i ) != NPos )
	{
		SizeType j = pos - i;

		// and back to the usual dull test
		StringChar ch = c_str()[j];
		if ( str.find( ch, 0, mode) == NPos)
			return j;
		i++;
	}
	return NPos;
}
//-----------------------------------------------------------------------------

String& String::insert(SizeType pos, const StringChar *str)
{
   return insert( pos, str, (uint32)strlen(str) );
}

///@todo review for error checking
String& String::insert(SizeType pos, const StringChar *str, SizeType len)
{
   if( !len )
      return *this;
         
   SizeType lena = length();
   uint32 newlen = lena + len;

   StringData *sub;
   if( !newlen )
      sub = StringData::Empty();
   else
   {
      sub = new ( newlen ) StringData( (StringChar*)NULL );

      String::copy(sub->utf8(),_string->utf8(),pos);
      String::copy(sub->utf8() + pos,str,len);
      String::copy(sub->utf8() + pos + len,_string->utf8() + pos,lena - pos + 1);
   }

   _string->release();
   _string = sub;

   return *this;
}

String& String::append(const StringChar *str, SizeType len)
{
	if( !len )
		return *this;

	SizeType lena = length();
	uint32 newlen = lena + len;

	StringData *sub;
	if( !newlen )
		sub = StringData::Empty();
	else
	{
		sub = new ( newlen ) StringData( (StringChar*)NULL );

		String::copy(sub->utf8(),_string->utf8(),lena);
		String::copy(sub->utf8() + lena, str, len+1);
	}

	_string->release();
	_string = sub;

	return *this;
}

String& String::append(const StringChar *str)
{
	return append(str,dStrlen(str));
}

String& String::append(const String &str)
{
	return append(str._string->utf8());
}

String& String::assign(SizeType num, const StringChar c)
{
	StringData *sub;
	if( !num )
		sub = StringData::Empty();
	else
	{
		sub = new ( num+1 ) StringData( (StringChar*)NULL );
		
		UTF8* p = sub->utf8();
		while(num--)
			*p++ = c;
		*p = 0;
	}

	_string->release();
	_string = sub;

	return *this;
}
String& String::erase(SizeType pos, SizeType len)
{
   if( !len )
      return *this;

   SizeType slen = length();
   uint32 newlen = slen - len;

   StringData *sub;
   if( !newlen )
      sub = StringData::Empty();
   else
   {
      sub = new ( newlen ) StringData( (StringChar*)NULL );

      if (pos > 0)
         String::copy(sub->utf8(),_string->utf8(),pos);

      String::copy(sub->utf8() + pos, _string->utf8() + pos + len, slen - (pos + len) + 1);
   }

   _string->release();
   _string = sub;

   return *this;
}

///@todo review for error checking
String& String::replace( SizeType pos, SizeType len, const StringChar *str )
{
   SizeType slen = length();
   SizeType rlen = (uint32)strlen(str);

   uint32 newlen = slen - len + rlen;
   StringData *sub;
   if( !newlen )
      sub = StringData::Empty();
   else
   {
      sub = new ( newlen ) StringData( (StringChar*)NULL );

      String::copy(sub->utf8(),_string->utf8(), pos);
      String::copy(sub->utf8() + pos,str,rlen);
      String::copy(sub->utf8() + pos + rlen,_string->utf8() + pos + len,slen - pos - len + 1);
   }

   _string->release();
   _string = sub;

   return *this;
}

String& String::replace( StringChar c1, StringChar c2 )
{
   if ( isEmpty() )
      return *this;

   // Create the new string lazily so that we don't needlessly
   // dup strings when there is nothing to replace.
   StringData* sub = NULL;   
   bool foundReplacement = false;
   StringChar* c = _string->utf8();
   while ( *c )
   {
      if ( *c == c1 )
      {
         if ( !foundReplacement )
         {
            sub = new ( length() ) StringData( _string->utf8() );
            c = &sub->utf8()[ c - _string->utf8() ];
            foundReplacement = true;
         }
         *c = c2;
      }
      c++;
   }
   if ( foundReplacement )
   {
      _string->release();
      _string = sub;
   }
   return *this;
}


String &String::replace( const String &s1, const String &s2 )
{
   // Find number of occurrences of s1 and
   // Calculate length of the new string...

   const uint32 &s1len = s1.length();
   const uint32 &s2len = s2.length();

   uint32 pos = 0;
   std::vector<uint32> indices;
   StringChar *walk = _string->utf8();

   while ( walk )
   {
      // Casting away the const... was there a better way?
      walk = (StringChar*)StrFind( _string->utf8(), s1.c_str(), pos, Case|Left );
      if ( walk )
      {
         pos = SizeType(walk - _string->utf8());
         indices.push_back( pos );
         pos += s1len;
      }
   }

   // Early-out, no StringDatas found.
   if ( indices.size() == 0 )
      return *this;

   uint32 newSize = size() - ( (uint32)indices.size() * s1len ) + ( (uint32)indices.size() * s2len );
   StringData *sub;
   if( newSize == 1 )
      sub = StringData::Empty();
   else
   {
      sub = new (newSize - 1 ) StringData( (StringChar*)NULL );

      // Now assemble the new string from the pieces of the old...

      // Index into the old string
      pos = 0;
      // Index into the new string
      uint32 newPos = 0;
      // Used to store a character count to be memcpy'd
      uint32 copyCharCount = 0;

      for ( uint32 i = 0; i < indices.size(); i++ )
      {
         const uint32 &index = indices[i];

         // Number of chars (if any) before the next indexed StringData
         copyCharCount = index - pos;

         // Copy chars before the StringData if we have any.
         if ( copyCharCount > 0 )
         {
            memcpy( sub->utf8() + newPos, _string->utf8() + pos, copyCharCount * sizeof(StringChar) );
            newPos += copyCharCount;
         }

         // Copy over the replacement string.
         if ( s2len > 0 )
            memcpy( sub->utf8() + newPos, s2._string->utf8(), s2len * sizeof(StringChar) );

         newPos += s2len;
         pos = index + s1len;
      }

      // There could be characters left in the original string after the last
      // StringData occurrence, which we need to copy now - outside the loop.
      copyCharCount = length() - indices.back() - s1len;
      if ( copyCharCount != 0 )
         memcpy( sub->utf8() + newPos, _string->utf8() + pos, copyCharCount * sizeof(StringChar) );

      // Null terminate it!
      sub->utf8()[newSize-1] = 0;
   }

   _string->release();
   _string = sub;

   return *this;
}

//-----------------------------------------------------------------------------
String String::substr(SizeType pos, SizeType len) const
{
   if ( len == -1 )
      len = length() - pos;

   StringData* sub;
   if( !len )
      sub = StringData::Empty();
   else
      sub = new ( len ) StringData( _string->utf8() + pos );
   return sub;
}


String String::subChar( SizeType pos, SizeType len ) const
{
	SizeType num = numChars();
	if (pos >= num)
		len = 0;
	else if ( len == -1)
		len = num - pos;
	else if ( pos + len > num)
		len = num - pos;

	//how get the size?
	StringData* sub;
	if( !len)
		sub = StringData::Empty();
	else
	{
		//int resultLen = dStrlen(convertEnc("UTF-8", "GBK",  convertUTF16toUTF8(  _string->utf16() + pos) )) 
		//					- dStrlen(convertEnc("UTF-8", "GBK",  convertUTF16toUTF8(  _string->utf16() + pos + len) ));	
		UTF8* tmp1 = WCharToMByte(_string->utf16() + pos);
		UTF8* tmp2 = WCharToMByte(_string->utf16() + pos + len) ;
		int resultLen = dStrlen(tmp1) - dStrlen(tmp2);	
		sub = new ( resultLen ) StringData( _string->utf16()+pos );
		delete tmp1;
		delete tmp2;
	}

	return sub;
}

UTF16 String::getChar( SizeType pos) const
{
	return _string->getChar(pos);
}

//-----------------------------------------------------------------------------
String String::trim() const
{
   if( isEmpty() )
      return *this;
   
   const StringChar* start = _string->utf8();
   while( *start && isspace( *start ) )
      start ++;
   
   const StringChar* end = _string->utf8() + length() - 1;
   while( end > start && isspace( *end ) )
      end --;
   end ++;
   
   const uint32 len = (uint32)(end-start);
   if ( len == length() )
      return *this;
   
   StringData* sub;
   if ( !len )
      sub = StringData::Empty();
   else
      sub = new ( len ) StringData( start );
   return sub;
}

//-----------------------------------------------------------------------------
String String::expandEscapes() const
{
   char* tmp = ( char* ) malloc( length() * 2 + 1 ); // worst-case situation.
   expandEscape( tmp, c_str() );
   String str( tmp );
   free( tmp );
   return str;
}

//-----------------------------------------------------------------------------
String String::collapseEscapes() const
{
   char* tmp = strdup( c_str() );
   collapseEscape( tmp );
   String str( tmp );
   free( tmp );
   return str;
}

//-----------------------------------------------------------------------------
void String::split( const char* delimiter, std::vector< String >& outElements ) const
{
	const char* ptr = _string->utf8();
	const char* start = ptr;
	while ( *ptr )
	{
		// Search for start of delimiter.
		if( *ptr != delimiter[ 0 ] )
			ptr ++;
		else
		{
			// Skip delimiter.
			const char* end = ptr;
			const char* del = delimiter;
			while( *del && *del == *ptr )
			{
				ptr ++;
				del ++;
			}
			// If we didn't match all of delimiter,
			// continue with search.
			if( *del != '\0' )
				continue;
			// Extract component.
			outElements.push_back( String(start,(uint32)(end-start)) );
			start = ptr;
		}
	}
	// Add rest of string if there is any.
	if ( start != ptr )
		outElements.push_back( start );
}

//-----------------------------------------------------------------------------
bool String::startsWith( const char* text ) const
{
#ifdef WINDOWS
   return !strnicmp( _string->utf8(), text, strlen(text) );
#else
    return !strncmp( _string->utf8(), text, strlen(text) );
#endif
}

//-----------------------------------------------------------------------------
bool dStrEndsWith( const char* str1, const char* str2 )
{
	const char *p = str1 + dStrlen(str1) - dStrlen(str2);
	return ((p >= str1) && !dStricmp(p, str2));
}


bool String::endsWith( const char* text ) const
{
	return dStrEndsWith( _string->utf8(), text );
}

//-----------------------------------------------------------------------------
void String::copy(StringChar* dst, const StringChar *src, uint32 len)
{
   memcpy( dst, src, len * sizeof(StringChar) );
}


//-----------------------------------------------------------------------------
//#define vsnprintf	_vsnprintf
String::StrFormat::~StrFormat()
{
   if( _dynamicBuffer )
      free( _dynamicBuffer );
}

int32 String::StrFormat::format( const char *format, void *args )
{
   _len=0;
   return formatAppend(format,args);
}


int32 String::StrFormat::formatAppend( const char *format, void *args )
{
	// Format into the fixed buffer first.
	int32 startLen = _len;
	if ( _dynamicBuffer == NULL )
	{
		_len += vsnprintf( _fixedBuffer+_len, sizeof(_fixedBuffer)-_len, format, *(va_list*)args );
		if ( _len>=0 && _len<sizeof(_fixedBuffer) )
			return _len;

		// Start off the dynamic buffer at twice fixed buffer size
		_len = startLen;
		_dynamicSize = sizeof(_fixedBuffer) * 2;
		_dynamicBuffer = (char*)malloc( _dynamicSize );
		memcpy( _dynamicBuffer, _fixedBuffer, _len + 1 );
	}

	// Format into the dynamic buffer, if the buffer is not large enough, then
	// keep doubling it's size until it is.  The buffer is not reallocated
	// using reallocate() to avoid unnecessary buffer copying.
	_len += vsnprintf(_dynamicBuffer + _len, _dynamicSize - _len, format, *(va_list*)args);
	while (_len < 0 || _len >= _dynamicSize)
	{
		_len = startLen;
		_dynamicBuffer = (char*)realloc(_dynamicBuffer, _dynamicSize *= 2);
		_len += vsnprintf(_dynamicBuffer + _len, _dynamicSize - _len, format, *(va_list*)args);
	}

	return _len;
}

int32 String::StrFormat::append(const char * str, int32 len)
{
   if (_dynamicBuffer == NULL)
   {
      if (_len+len >= 0 && _len+len < sizeof(_fixedBuffer))
      {
         memcpy(_fixedBuffer + _len, str, len);
         _len += len;
         _fixedBuffer[_len] = '\0';
         return _len;
      }

      _dynamicSize = sizeof(_fixedBuffer) * 2;
      _dynamicBuffer = (char*)malloc(_dynamicSize);
      memcpy(_dynamicBuffer, _fixedBuffer, _len + 1);
   }

   uint32 newSize = _dynamicSize;
   while (newSize < (uint32)_len+len)
      newSize *= 2;
   if ( newSize != _dynamicSize )
      _dynamicBuffer = (char*) realloc(_dynamicBuffer, newSize);
   _dynamicSize = newSize;
   memcpy(_dynamicBuffer + _len, str, len);
   _len += len;
   _dynamicBuffer[_len] = '\0';
   return _len;
}

int32 String::StrFormat::append(const char * str)
{
   return append( str, (int32)strlen(str) );
}

char* String::StrFormat::copy( char *buffer ) const
{
   memcpy(buffer, _dynamicBuffer? _dynamicBuffer: _fixedBuffer, _len+1);
   return buffer;
}

//-----------------------------------------------------------------------------

String String::ToString( bool value )
{
   static String sTrue = "true";
   static String sFalse = "false";
   
   if( value )
      return sTrue;
   return sFalse;
}

String String::ToString(const char *str, ...)
{
   // Use the format object
   va_list args;
   va_start(args, str);
#ifndef __ANDROID__
   String ret = VToString(str, args);
   va_end(args);
   return ret;
#else
   char buff[1024];
	va_list va_alist;
	va_start(va_alist, str);
	vsprintf(buff, str, va_alist);
	va_end(va_alist);
   return buff;
#endif
}

String String::VToString(const char* str, void* args)
{
   StrFormat format(str,&args);

   // Copy it into a string
   uint32         len = format.length();
   StringData* sub;
   if( !len )
      sub = StringData::Empty();
   else
   {
      sub = new ( len ) StringData( (StringChar*)NULL );

      format.copy( sub->utf8() );
      sub->utf8()[ len ] = 0;
   }

   return sub;
}

String   String::SpanToString(const char *start, const char *end)
{
   if ( end == start )
      return String();

   uint32         len = uint32(end - start);
   StringData* sub = new ( len ) StringData( start );

   return sub;
}

String String::ToLower(const String &string)
{
   if ( string.isEmpty() )
      return String();

   StringData* sub = new ( string.length() ) StringData( string.c_str() );
   strlwr( sub->utf8() );

   return sub;
}

String String::ToUpper(const String &string)
{
   if ( string.isEmpty() )
      return String();

   StringData* sub = new ( string.length() ) StringData( string.c_str() );
   strupr( sub->utf8() );
   return sub;
}

String String::GetTrailingNumber(const char* str, int32& number)
{
	// Check for trivial strings
	if ( !str || !str[0] )
		return String::EmptyString;

	// Find the number at the end of the string
	String base( str );
	const char* p = base.c_str() + base.length() - 1;

	// Ignore trailing whitespace
	while ( (p != base.c_str()) && isspace(*p) )
		p--;

	// Need at least one digit!
	if ( !isdigit(*p) )
		return base;

	// Back up to the first non-digit character
	while ( (p != base.c_str()) && isdigit(*p) )
		p--;

	// Convert number => allow negative numbers, treat '_' as '-' for Maya
	if ( (*p == '-') || (*p == '_') )
		number = -atoi(p + 1);
	else
		number = ( (p == base.c_str()) ? atoi(p) : atoi(++p) );

	// Remove space between the name and the number
	while ( (p > base.c_str()) && isspace(*(p-1)) )
		p--;

	return base.substr( 0, (uint32)(p-base.c_str()) );
}

String String::getFileName()
{
	SizeType pos1 = find( '/', 0, String::Right );
	SizeType pos2 = find( '\\', 0, String::Right );
	int iPos1 = pos1;
	int iPos2 = pos2;
	if ( pos1 == NPos )
		iPos1 = -1;
	if ( pos2 == NPos )
		iPos2 = -1;
	int pos =  __max( iPos1, iPos2 );

	if ( pos >= 0 )
	{
		if ( length()-pos-1 > 0 )
		{
			StringData* sub = new ( length()-pos-1 ) StringData( _string->utf8()+pos+1 );
			return sub;
		}
	}	
	return EmptyString;
}


String String::getFilePath()
{
	SizeType pos1 = find( '/', 0, String::Right );
	SizeType pos2 = find( '\\', 0, String::Right );
	int iPos1 = pos1;
	int iPos2 = pos2;
	if ( pos1 == NPos )
		iPos1 = -1;
	if ( pos2 == NPos )
		iPos2 = -1;
	int pos =  __max( iPos1, iPos2 );

	if ( pos >= 0 )
	{
		StringData* sub = new ( pos+1 ) StringData( _string->utf8() );
		return sub;
	}	
	return EmptyString;
}


void String::makePath()
{
	if ( isEmpty() )
		return;
	char ch = operator[]( length()-1 );
	if ( ch=='\\' || ch =='/' )
		return;
	operator += ( '/' );
}

void String::formatPath()
{
	char* pStr = _string->utf8();
	strlwr( pStr );
	uint32 len = _string->getLength();
	for ( uint32 i=0; i<len; i++ )
	{
		if ( pStr[i] == '\\' )
			pStr[i] = '/';
	}
	String::SizeType pos;
	while ( (pos=find("//")) != String::NPos )
		replace( pos, 2, "/" );
}
