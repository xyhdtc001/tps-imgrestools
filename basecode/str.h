#ifndef _MY_STRING_H_
#define _MY_STRING_H_

#include <cstdarg>
#include <vector>

#include "basecode/basetype.h"
//#include "Windows.h"

typedef UTF8 StringChar;

/// The String class represents a 0-terminated array of characters.
class String
{
public:
	class StringData;

	/// Default mode is case sensitive starting from the left
	enum Mode
	{
		Case = 0,         ///< Case sensitive
		NoCase = 1,       ///< Case insensitive
		Left = 0,         ///< Start at left end of string
		Right = 2,        ///< Start at right end of string
	};

	typedef uint32 SizeType;
	typedef StringChar ValueType;

	static const SizeType NPos;   ///< Indicates 'not found' when using find() functions

	/// A predefined empty string.
	static const String EmptyString;

	String();
	String(const String &str);
	String(const StringChar *str);
	String(const StringChar *str, SizeType size);
	String(const UTF16 *str);
	String(SizeType num, const StringChar c);
	~String();

	int32 toInt32();
	ulong toULong(int base);

	float32 toFloat32();

	int64 toInt64();

	const UTF8  *c_str() const;   ///< Return the string as a native type
	const UTF16 *utf16() const;
	const UTF8* utf8() const { return c_str(); }

	SizeType length() const;   ///< Returns the length of the string in bytes.
	SizeType size() const;     ///< Returns the length of the string in bytes including the NULL terminator.
	bool     isEmpty() const;  ///< Is this an empty string [""]?
	bool     isNotEmpty() const { return !isEmpty(); }  ///< Is this not an empty string [""]?

	/// Erases all characters in a string.
	void clear() { *this = EmptyString; }

	bool     isShared() const; ///< Is this string's reference count greater than 1?
	bool     isSame( const String& str ) const; ///< Return true if both strings refer to the same shared data.

	//uint32 getHashCaseSensitive() const;    ///< Get the case-sensitive hash of the string [only calculates the hash as necessary]
	//uint32 getHashCaseInsensitive() const;  ///< Get the case-insensitive hash of the string  [only calculates the hash as necessary]

	String& operator=(StringChar);
	String& operator+=(StringChar);
	String& operator=(const StringChar*);
	String& operator+=(const StringChar*);
	String& operator=(const String&);
	String& operator+=(const String&);


	bool match( const String& match, String* retModifiedData, bool bReplace = false );
	void removeChar( char ch );
	void removeSpaces();
	void removeCRLF();
	void removeTab();
	String getFileName();
	String getFilePath();
	void makePath();
	void formatPath();

	bool isEndWith( const String& str, uint32 mode=String::Case ) const;
	int32 HexToInt();

	/**
	Compare this string with another.
	@param str  The string to compare against.
	@param len  If len is non-zero, then at most len characters are compared.
	@param mode Comparison mode.
	@return Difference between the first two characters that don't match.
	*/


	int32 compare(const StringChar *str, SizeType len = 0, uint32 mode = Case|Left) const;
	int32 compare(const String &str, SizeType len = 0, uint32 mode = Case|Left) const; ///< @see compare(const StringChar *, SizeType, uint32) const

	/**
	Compare two strings for equality.
	It will use the string hashes to determine inequality.
	@param str  The string to compare against.
	@param mode Comparison mode - case sensitive or not.
	*/
	bool equal(const String &str, uint32 mode = Case) const;

	SizeType find(StringChar c, SizeType pos = 0, uint32 mode = Case|Left) const;
	SizeType find(const StringChar *str, SizeType pos = 0, uint32 mode = Case|Left) const;
	SizeType find(const String &str, SizeType pos = 0, uint32 mode = Case|Left) const;
	SizeType findFirst(const String &str, SizeType pos = 0, uint32 mode = Case|Left) const;
	SizeType findLast(const String &str, SizeType pos = NPos, uint32 mode = Case|Left) const;

	SizeType findFirstNot(const String &str, SizeType pos = 0, uint32 mode = Case|Left) const;
	SizeType findLastNot(const String &str, SizeType pos = NPos, uint32 mode = Case|Left) const;

	String   &insert(SizeType pos, const StringChar c) { return insert(pos,&c,1); }
	String   &insert(SizeType pos, const StringChar *str);
	String   &insert(SizeType pos, const String &str);
	String   &insert(SizeType pos, const StringChar *str, SizeType len);

	String   &append(const StringChar c) { return append(&c,1); }
	String	&append(const StringChar *str);
	String   &append(const String &str);  
	String   &append(const StringChar *str, SizeType len);

	String	&assign(SizeType num, const StringChar c);
	String   &erase(SizeType pos, SizeType len);

	String   &replace(SizeType pos, SizeType len, const StringChar *str);
	String   &replace(SizeType pos, SizeType len, const String &str);

	/// Replace all occurrences of character 'c1' with 'c2'
	String &replace( StringChar c1, StringChar c2 );

	/// Replace all occurrences of StringData 's1' with StringData 's2'
	String &replace(const String &s1, const String &s2);

	String substr( SizeType pos, SizeType len = -1 ) const;

	SizeType numChars() const; 
	String subChar( SizeType pos, SizeType len = -1 ) const;
	UTF16 getChar( SizeType pos) const;

	/// Remove leading and trailing whitespace.
	String trim() const;

	/// Replace all characters that need to be escaped for the string to be a valid string literal with their
	/// respective escape sequences.
	String expandEscapes() const;

	/// Replace all escape sequences in with their respective character codes.
	String collapseEscapes() const;

	/// Split the string into its components separated by the given delimiter.
	void split( const char* delimiter, std::vector< String >& outElements ) const;

	/// Return true if the string starts with the given text.
	bool startsWith( const char* text ) const;

	/// Return true if the string ends with the given text.
	bool endsWith( const char* text ) const;

	StringChar operator []( uint32 i ) const { return c_str()[i]; }
	StringChar operator []( int32 i ) const { return c_str()[i]; }

	bool operator==(const StringChar* str) const;
	bool operator==(const String &str) const;
	bool operator!=(const String &str) const { return !(*this == str); }
	bool operator!=(const char* str) const { return !(*this == String(str)); }
	bool operator==( StringChar c ) const;
	bool operator!=( StringChar c ) const { return !(*this == c); }
	bool operator<(const String &str) const;
	bool operator>(const String &str) const;
	bool operator<=(const String &str) const;
	bool operator>=(const String &str) const;

	friend String operator+(const String &a, StringChar c);
	friend String operator+(StringChar c, const String &a);
	friend String operator+(const String &a, const StringChar *b);
	friend String operator+(const String &a, const String &b);
	friend String operator+(const StringChar *a, const String &b);

public:


public:
	/// @name String Utility routines
	/// @{

	static String ToString(const char *format, ...);
	static String VToString(const char* format, void* args);

	static String ToString( bool v );
	static inline String ToString( uint32 v ) { return ToString( "%u", v ); }
	static inline String ToString( int32 v ) { return ToString( "%d", v ); }
	static inline String ToString( float32 v ) { return ToString( "%g", v ); }
	static inline String ToString( float64 v ) { return ToString( "%Lg", v ); }

	static String SpanToString(const char* start, const char* end);

	static String ToLower(const String &string);
	static String ToUpper(const String &string);

	static String GetTrailingNumber(const char* str, int32& number);
	static SizeType max_size(void)
	{
		return (((SizeType)-1) / sizeof(ValueType));
	}

	class StrFormat
	{
	public:
		StrFormat()
			:  _dynamicBuffer( NULL ),
			_dynamicSize( 0 ),
			_len( 0 )
		{
			_fixedBuffer[0] = '\0';
		}

		StrFormat(const char *formatStr, void *args)
			:  _dynamicBuffer( NULL ),
			_dynamicSize( 0 ),
			_len( 0 )
		{
			format(formatStr, args);
		}

		~StrFormat();

		int32 format( const char *format, void *args );
		int32 formatAppend( const char *format, void *args );
		int32 append(const char * str, int32 len);
		int32 append(const char * str);

		String getString() { return String(c_str(),_len); }

		const char * c_str() const { return _dynamicBuffer ? _dynamicBuffer : _fixedBuffer; }

		void reset()
		{
			_len = 0;
			_fixedBuffer[0] = '\0';
		}

		/// Copy the formatted string into the output buffer which must be at least size() characters.
		char  *copy(char* buffer) const;

		/// Return the length of the formated string (does not include the terminating 0)
		uint32 length() const { return _len; };

	public:
		char  _fixedBuffer[2048];  //< Fixed size buffer
		char  *_dynamicBuffer;     //< Temporary format buffer
		uint32  _dynamicSize;        //< Dynamic buffer size
		uint32  _len;                //< Len of the formatted string
	};

private:
	String(StringData *str)
		: _string( str ) {}

	static void copy(StringChar *dst, const StringChar *src, uint32 size);

	StringData   *_string;
};

// Utility class for formatting strings.
class StringBuilder
{
   protected:

      ///
      String::StrFormat mFormat;

   public:

      StringBuilder() {}
      
      uint32 length() const
      {
         return mFormat.length();
      }
      
      void copy( char* buffer ) const
      {
         mFormat.copy( buffer );
      }

      const char* data() const
      {
         return mFormat.c_str();
      }

      String end()
      {
         return mFormat.getString();
      }

      int32 append( char ch )
      {
         char str[2];
         str[0]=ch;
         str[1]='\0';
         return mFormat.append(str);
      }
      int32 append( const char* str )
      {
         return mFormat.append(str);
      }
      int32 append( const String& str )
      {
         return mFormat.append( str.c_str(), str.length() );
      }
      int32 append( const char* str, uint32 length )
      {
         return mFormat.append(str,length);
      }
      int32 format( const char* fmt, ... )
      {
         va_list args;
         va_start(args, fmt);
         return mFormat.formatAppend(fmt, &args);
      }
};

// For use in hash tables and the like for explicitly requesting case sensitive hashing.
// Meant to only appear in hash table definition (casting will take care of the rest).
class StringCase : public String
{
public:
   StringCase() : String() {}
   StringCase(const String & s) : String(s) {}
};

// For use in hash tables and the like for explicitly requesting case insensitive hashing.
// Meant to only appear in hash table definition (casting will take care of the rest).
class StringNoCase : public String
{
public:
   StringNoCase() : String() {}
   StringNoCase(const String & s) : String(s) {}
};

class FileName : public String
{
public:
   FileName() : String() {}
   FileName(const String & s) : String(s) {}
   FileName & operator=(const String & s) { String::operator=(s); return *this; }
};

//-----------------------------------------------------------------------------

extern String operator+(const String &a, StringChar c);
extern String operator+(StringChar c, const String &a);
extern String operator+(const String &a, const StringChar *b);
extern String operator+(const String &a, const String &b);
extern String operator+(const StringChar *a, const String &b);

#endif

