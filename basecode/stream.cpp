//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------
#include <string.h>
#include "basecode/stream.h"
#include "basecode/endian.h"
//#include "basecode/string/frameAllocator.h"
//#include "basecode/string/stringFunctions.h"
#include "basecode/str.h"


#define IMPLEMENT_OVERLOADED_READ(type)      \
   bool Stream::read(type* out_read)         \
   {                                         \
      return read(sizeof(type), out_read);   \
   }

#define IMPLEMENT_OVERLOADED_WRITE(type)        \
   bool Stream::write(type in_write)            \
   {                                            \
      return write(sizeof(type), &in_write);    \
   }

#define IMPLEMENT_ENDIAN_OVERLOADED_READ(type)  \
   bool Stream::read(type* out_read)            \
   {                                            \
      type temp;                                \
      bool success = read(sizeof(type), &temp); \
      *out_read = convertLEndianToHost(temp);   \
      return success;                           \
   }

#define IMPLEMENT_ENDIAN_OVERLOADED_WRITE(type)    \
   bool Stream::write(type in_write)               \
   {                                               \
      type temp = convertHostToLEndian(in_write);  \
      return write(sizeof(type), &temp);           \
   }

IMPLEMENT_OVERLOADED_WRITE(int8)
IMPLEMENT_OVERLOADED_WRITE(uint8)

IMPLEMENT_ENDIAN_OVERLOADED_WRITE(int16)
IMPLEMENT_ENDIAN_OVERLOADED_WRITE(int32)
IMPLEMENT_ENDIAN_OVERLOADED_WRITE(uint16)
IMPLEMENT_ENDIAN_OVERLOADED_WRITE(uint32)
IMPLEMENT_ENDIAN_OVERLOADED_WRITE(uint64)
IMPLEMENT_ENDIAN_OVERLOADED_WRITE(float32)
IMPLEMENT_ENDIAN_OVERLOADED_WRITE(float64)

IMPLEMENT_OVERLOADED_READ(int8)
IMPLEMENT_OVERLOADED_READ(uint8)

IMPLEMENT_ENDIAN_OVERLOADED_READ(int16)
IMPLEMENT_ENDIAN_OVERLOADED_READ(int32)
IMPLEMENT_ENDIAN_OVERLOADED_READ(uint16)
IMPLEMENT_ENDIAN_OVERLOADED_READ(uint32)
IMPLEMENT_ENDIAN_OVERLOADED_READ(uint64)
IMPLEMENT_ENDIAN_OVERLOADED_READ(float32)
IMPLEMENT_ENDIAN_OVERLOADED_READ(float64)


Stream::Stream() : m_streamStatus(Closed)
{
}

const char* Stream::getStatusString(const Status in_status)
{
   switch (in_status)
   {
      case Ok:
         return "StreamOk";
      case IOError:
         return "StreamIOError";
      case EOS:
         return "StreamEOS";
      case IllegalCall:
         return "StreamIllegalCall";
      case Closed:
         return "StreamClosed";
      case UnknownError:
         return "StreamUnknownError";

     default:
      return "Invalid Stream::Status";
   }
}

void Stream::writeString(const char *string, int32 maxLen)
{
	int32 len = string ? (int)strlen(string) : 0;
	if(len > maxLen)
		len = maxLen;

	write(uint8(len));
	if(len)
		write(len, string);
}

void Stream::readString(char buf[256])
{
   uint8 len;
   read(&len);
   read(int32(len), buf);
   buf[len] = 0;
}


void Stream::readLongString(uint32 maxStringLen, char *stringBuf)
{
   uint32 len;
   read(&len);
   if(len > maxStringLen)
   {
      m_streamStatus = IOError;
      return;
   }
   read(len, stringBuf);
   stringBuf[len] = 0;
}

void Stream::writeLongString(uint32 maxStringLen, const char *string)
{
   uint32 len = (uint32)strlen(string);
   if(len > maxStringLen)
      len = maxStringLen;
   write(len);
   write(len, string);
}

void Stream::readLine( uint8 *buffer, uint32 bufferSize )
{
   bufferSize--;  // account for NULL terminator
   uint8 *buff = buffer;
   uint8 *buffEnd = buff + bufferSize;
   *buff = '\r';

   // strip off preceding white space
   while ( *buff == '\r' )
   {
      if ( !read(buff) || *buff == '\n' )
      {
         *buff = 0;
         return;
      }
   }

   // read line
   while ( buff != buffEnd && read(++buff) && *buff != '\n' )
   {
      if ( *buff == '\r' )
      {
		buff--; // 'erases' the CR of a CRLF
      }
   }
   *buff = 0;
}

void Stream::writeText( const char *text )
{
	if ( text && text[0] )
		write( (uint32)strlen(text), text );
}

void Stream::writeLine(const uint8 *buffer)
{
   write( (uint32)strlen((const char*)buffer), buffer );
   write( 2, "\r\n" );
}

void Stream::write( const String & str )
{
	uint32 len = str.length();
	if ( len<255 )
	{
		write( uint8(len) );
	}
	else
	{
		// longer string, write full length
		write( uint8(255) );
		len &= (1<<16)-1;
		write( uint16(len) );
	}
	write( len,str.c_str() );
}

/*
void Stream::_read(String * str)
{
   uint16 len;

   uint8 len8;
   read(&len8);
   if (len8==255)
      read(&len);
   else
      len = len8;

   char * buffer = (char*)FrameAllocator::alloc(len);
   read(len, buffer);
   *str = String(buffer,len);
}
*/


bool Stream::copyFrom(Stream *other)
{
   uint8 buffer[1024];
   uint32 numBytes = other->getStreamSize() - other->getWritePos();
   while((other->getStatus() != Stream::EOS) && numBytes > 0)
   {
      uint32 numRead = numBytes > sizeof(buffer) ? sizeof(buffer) : numBytes;
      if(! other->read(numRead, buffer))
         return false;

      if(! write(numRead, buffer))
         return false;

      numBytes -= numRead;
   }

   return true;
}

Stream* Stream::clone() const
{
   return NULL;
}
