
#include <stdio.h>

#include "basecode/frameAllocator.h"
#include "basecode/unicode.h"
#include "basecode/stringFunctions.h"

#include <Windows.h>

#define kReplacementChar 0xFFFD

static const uint8 sgFirstByteLUT[128] = 
{
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x0F // single byte ascii
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x1F // single byte ascii
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x2F // single byte ascii
   1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, // 0x3F // single byte ascii

   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x4F // trailing utf8
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x5F // trailing utf8
   2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2,  2, 2, 2, 2, // 0x6F // first of 2
   3, 3, 3, 3,  3, 3, 3, 3,  4, 4, 4, 4,  5, 5, 6, 0, // 0x7F // first of 3,4,5,illegal in utf-8
};


static const uint8 sgSurrogateLUT[64] = 
{
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x0F 
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x1F 
   0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, // 0x2F 
   0, 0, 0, 0,  0, 0, 1, 2,  0, 0, 0, 0,  0, 0, 0, 0, // 0x3F 
};


static const uint8  sgByteMask8LUT[]  = { 0x3f, 0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01 }; // last 0=6, 1=7, 2=5, 4, 3, 2, 1 bits

/// Mask for the data bits of a UTF-16 surrogate.
static const uint16 sgByteMaskLow10 = 0x03ff;



//#include "iconv/IConv.h"

#ifndef WINDOWS
#ifndef __ANDROID__
#include <iconv.h>
#endif
const int ICONV_BUFFER_SIZE = 10240;
UTF8* ConvertEnc( const UTF8* encFrom, const UTF8* encTo, const UTF8* in)
{
#ifndef __ANDROID__
	static UTF8  bufout[ICONV_BUFFER_SIZE], *sout;
	const UTF8* sin;

	int  lenin, lenout, ret;
	iconv_t c_pt = iconv_open(encTo, encFrom);

	if (c_pt == (iconv_t)-1)
	{
		return NULL;
	}
	iconv(c_pt, NULL, NULL, NULL, NULL);

	lenin  = dStrlen(in) + 1;
	lenout = ICONV_BUFFER_SIZE;

	sin    = (const UTF8 *)in;
	sout   = bufout;

	ret = iconv(c_pt, (char**)&sin, (size_t *)&lenin, (char**)&sout, (size_t *)&lenout);
	if (ret == -1)
	{
		return NULL;
	}

	//UTF8* result = new UTF8[lenout];
	//memcpy(result, bufout, lenout);

	iconv_close(c_pt);
	return bufout;
#else
	return "";
#endif
}
#endif



UTF16* MByteToWChar(const UTF8* lpcszStr)
{	
#ifdef WINDOWS
	//uint32 len = dStrlen(lpcszStr) + 1;
	//static UTF16 buf[2048] = {0};

	uint32 nCodepoints = MultiByteToWideChar (CP_UTF8, 0, lpcszStr, -1, NULL, 0);

	UTF16 *ret = new UTF16[nCodepoints];
	MultiByteToWideChar (CP_UTF8, 0, lpcszStr, -1, (LPWSTR)(ret), nCodepoints);  
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

	uint32 nCodeunits = WideCharToMultiByte(CP_UTF8,NULL, (LPWSTR)lpcwszStr, -1, NULL, 0, NULL, FALSE);

	UTF8 *ret = new UTF8[nCodeunits];
	WideCharToMultiByte(CP_UTF8,NULL,(LPWSTR)lpcwszStr, -1, ret, nCodeunits,NULL,FALSE);
	//memcpy(ret, buf, nCodeunits * sizeof(UTF8));
	return ret;
#else
	UTF8* result = convertUTF16toUTF8( lpcwszStr);
	UTF8* temp = ConvertEnc("UTF-8", "GBK",  result);
	uint32 len = dStrlen(result) + 1;
	memcpy(result, temp, len);
	return result;
#endif
}



///-----------------------------------------------------------------------------
inline bool isSurrogateRange(uint32 codepoint)
{
   return ( 0xd800 < codepoint && codepoint < 0xdfff );
}

inline bool isAboveBMP(uint32 codepoint)
{
   return ( codepoint > 0xFFFF );
}

//-----------------------------------------------------------------------------
uint32 convertUTF8toUTF16(const UTF8 *unistring, UTF16 *outbuffer, uint32 len)
{
   uint32 walked, nCodepoints;
   UTF32 middleman;
   
   nCodepoints=0;
   while(*unistring != '\0' && nCodepoints < len)
   {
      walked = 1;
      middleman = oneUTF8toUTF32(unistring,&walked);
      outbuffer[nCodepoints] = oneUTF32toUTF16(middleman);
      unistring+=walked;
      nCodepoints++;
   }

   nCodepoints = min(nCodepoints,len - 1);
   outbuffer[nCodepoints] = '\0';   
   return nCodepoints; 
}

//-----------------------------------------------------------------------------
uint32 convertUTF16toUTF8( const UTF16 *unistring, UTF8  *outbuffer, uint32 len)
{
   uint32 walked, nCodeunits, codeunitLen;
   UTF32 middleman;
   
   nCodeunits=0;
   while( *unistring != '\0' && nCodeunits + 3 < len )
   {
      walked = 1;
      middleman  = oneUTF16toUTF32(unistring,&walked);
      codeunitLen = oneUTF32toUTF8(middleman, &outbuffer[nCodeunits]);
      unistring += walked;
      nCodeunits += codeunitLen;
   }

   nCodeunits = min(nCodeunits,len - 1);
   outbuffer[nCodeunits] = '\0';
      return nCodeunits;
}

uint32 convertUTF16toUTF8DoubleNULL( const UTF16 *unistring, UTF8  *outbuffer, uint32 len)
{
   uint32 walked, nCodeunits, codeunitLen;
   UTF32 middleman;

   nCodeunits=0;
   while( ! (*unistring == '\0' && *(unistring + 1) == '\0') && nCodeunits + 3 < len )
   {
      walked = 1;
      middleman  = oneUTF16toUTF32(unistring,&walked);
      codeunitLen = oneUTF32toUTF8(middleman, &outbuffer[nCodeunits]);
      unistring += walked;
      nCodeunits += codeunitLen;
   }

   nCodeunits = min(nCodeunits,len - 1);
   outbuffer[nCodeunits] = NULL;
   outbuffer[nCodeunits+1] = NULL;

   return nCodeunits;
}

//-----------------------------------------------------------------------------
// Functions that convert buffers of unicode code points
//-----------------------------------------------------------------------------
UTF16* convertUTF8toUTF16( const UTF8* unistring)
{ 
   // allocate plenty of memory.
   uint32 nCodepoints, len = dStrlen(unistring) + 1;
   FrameTemp<UTF16> buf(len);
   
   // perform conversion
   nCodepoints = convertUTF8toUTF16( unistring, buf, len);
   
   // add 1 for the NULL terminator the converter promises it included.
   nCodepoints++;
   
   // allocate the return buffer, copy over, and return it.
   UTF16 *ret = new UTF16[nCodepoints];
   memcpy(ret, buf, nCodepoints * sizeof(UTF16));
   
   return ret;
}

//-----------------------------------------------------------------------------
UTF8*  convertUTF16toUTF8( const UTF16* unistring)
{
   // allocate plenty of memory.
   uint32 nCodeunits, len = dStrlen(unistring) * 3 + 1;
   FrameTemp<UTF8> buf(len);
      
   // perform conversion
   nCodeunits = convertUTF16toUTF8( unistring, buf, len);
   
   // add 1 for the NULL terminator the converter promises it included.
   nCodeunits++;
   
   // allocate the return buffer, copy over, and return it.
   UTF8 *ret = new UTF8[nCodeunits];
   memcpy(ret, buf, nCodeunits * sizeof(UTF8));

   return ret;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions that converts one unicode codepoint at a time
//-----------------------------------------------------------------------------
UTF32 oneUTF8toUTF32( const UTF8* codepoint, uint32 *unitsWalked)
{  
   // early out for ascii
   if(!(*codepoint & 0x0080))
   {
      if (unitsWalked != NULL)
         *unitsWalked = 1;
      return (UTF32)*codepoint;
   }
   
   uint32 expectedByteCount;
   UTF32  ret = 0;
   uint8 codeunit;
   
   // check the first byte ( a.k.a. codeunit ) .
   unsigned char c = codepoint[0];
   c = c >> 1;
   expectedByteCount = sgFirstByteLUT[c];
   if(expectedByteCount > 0) // 0 or negative is illegal to start with
   {
      // process 1st codeunit
      ret |= sgByteMask8LUT[expectedByteCount] & codepoint[0]; // bug?
      
      // process trailing codeunits
      for(uint32 i=1;i<expectedByteCount; i++)
      {
         codeunit = codepoint[i];
         if( sgFirstByteLUT[codeunit>>1] == 0 )
         {
            ret <<= 6;                 // shift up 6
            ret |= (codeunit & 0x3f);  // mask in the low 6 bits of this codeunit byte.
         }
         else
         {
            // found a bad codepoint - did not get a medial where we wanted one.
            // Dump the replacement, and claim to have parsed only 1 char,
            // so that we'll dump a slew of replacements, instead of eating the next char.            
            ret = kReplacementChar;
            expectedByteCount = 1;
            break;
         }
      }
   }
   else 
   {
      // found a bad codepoint - got a medial or an illegal codeunit. 
      // Dump the replacement, and claim to have parsed only 1 char,
      // so that we'll dump a slew of replacements, instead of eating the next char.
      ret = kReplacementChar;
      expectedByteCount = 1;
   }
   
   if(unitsWalked != NULL)
      *unitsWalked = expectedByteCount;
   
   // codepoints in the surrogate range are illegal, and should be replaced.
   if(isSurrogateRange(ret))
      ret = kReplacementChar;
   
   // codepoints outside the Basic Multilingual Plane add complexity to our UTF16 string classes,
   // we've read them correctly so they won't foul the byte stream,
   // but we kill them here to make sure they wont foul anything else
   if(isAboveBMP(ret))
      ret = kReplacementChar;

   return ret;
}

//-----------------------------------------------------------------------------
UTF32  oneUTF16toUTF32(const UTF16* codepoint, uint32 *unitsWalked)
{
   uint8    expectedType;
   uint32   unitCount;
   UTF32 ret = 0;
   UTF16 codeunit1,codeunit2;
   
   codeunit1 = codepoint[0];
   expectedType = sgSurrogateLUT[codeunit1 >> 10];
   switch(expectedType)
   {
      case 0: // simple
         ret = codeunit1;
         unitCount = 1;
         break;
      case 1: // 2 surrogates
         codeunit2 = codepoint[1];
         if( sgSurrogateLUT[codeunit2 >> 10] == 2)
         {
            ret = ((codeunit1 & sgByteMaskLow10 ) << 10) | (codeunit2 & sgByteMaskLow10);
            unitCount = 2;
            break;
         }
         // else, did not find a trailing surrogate where we expected one,
         // so fall through to the error
      case 2: // error
         // found a trailing surrogate where we expected a codepoint or leading surrogate.
         // Dump the replacement.
         ret = kReplacementChar;
         unitCount = 1;
         break;
      default:
         // unexpected return
         ret = kReplacementChar;
         unitCount = 1;
         break;
   }

   if(unitsWalked != NULL)
      *unitsWalked = unitCount;


   if(isSurrogateRange(ret))
      ret = kReplacementChar;

   if(isAboveBMP(ret))
      ret = kReplacementChar;

   return ret;
}

//-----------------------------------------------------------------------------
UTF16 oneUTF32toUTF16(const UTF32 codepoint)
{
   // found a codepoint outside the encodable UTF-16 range!
   // or, found an illegal codepoint!
   if(codepoint >= 0x10FFFF || isSurrogateRange(codepoint))
      return kReplacementChar;
   
   // these are legal, we just don't want to deal with them.
   if(isAboveBMP(codepoint))
      return kReplacementChar;

   return (UTF16)codepoint;
}

//-----------------------------------------------------------------------------
uint32 oneUTF32toUTF8(const UTF32 codepoint, UTF8 *threeByteCodeunitBuf)
{
   uint32 bytecount = 0;
   UTF8 *buf;
   uint32 working = codepoint;
   buf = threeByteCodeunitBuf;

   //-----------------
   if(isSurrogateRange(working))  // found an illegal codepoint!
      working = kReplacementChar;
   
   if(isAboveBMP(working))        // these are legal, we just dont want to deal with them.
      working = kReplacementChar;

   //-----------------
   if( working < (1 << 7))        // codeable in 7 bits
      bytecount = 1;
   else if( working < (1 << 11))  // codeable in 11 bits
      bytecount = 2;
   else if( working < (1 << 16))  // codeable in 16 bits
      bytecount = 3;


   //-----------------
   uint8  mask = sgByteMask8LUT[0];            // 0011 1111
   uint8  marker = ( ~mask << 1);            // 1000 0000
   
   // Process the low order bytes, shifting the codepoint down 6 each pass.
   for( int i = bytecount-1; i > 0; i--)
   {
      threeByteCodeunitBuf[i] = marker | (working & mask); 
      working >>= 6;
   }

   // Process the 1st byte. filter based on the # of expected bytes.
   mask = sgByteMask8LUT[bytecount];
   marker = ( ~mask << 1 );
   threeByteCodeunitBuf[0] = marker | working & mask;
   
   return bytecount;
}

//-----------------------------------------------------------------------------
uint32 dStrlen(const UTF16 *unistring)
{
   if(!unistring)
      return 0;

   uint32 i = 0;
   while(unistring[i] != '\0')
      i++;
      
//   AssertFatal( wcslen(unistring) == i, "Incorrect length" );

   return i;
}

//-----------------------------------------------------------------------------
uint32 dStrlen(const UTF32 *unistring)
{
   uint32 i = 0;
   while(unistring[i] != '\0')
      i++;
      
   return i;
}

uint32 dStrlen(const UTF8 *str)
{
	return (uint32)strlen(str);
}   

//-----------------------------------------------------------------------------
uint32 dStrncmp(const UTF16* unistring1, const UTF16* unistring2, uint32 len)
{
   UTF16 c1, c2;
   for(uint32 i = 0; i<len; i++)
   {
      c1 = *unistring1++;
      c2 = *unistring2++;
      if(c1 < c2) return -1;
      if(c1 > c2) return 1;
      if(!c1) return 0;
   }
   return 0;
}

//-----------------------------------------------------------------------------

const UTF16* dStrrchr(const UTF16* unistring, uint32 c)
{
   if(!unistring) return NULL;

   const UTF16* tmp = unistring + dStrlen(unistring);
   while( tmp >= unistring)
   { 
      if(*tmp == c)
         return tmp;
      tmp--;
   }
   return NULL;
}

UTF16* dStrrchr(UTF16* unistring, uint32 c)
{
   const UTF16* str = unistring;
   return const_cast<UTF16*>(dStrrchr(str, c));
}

const UTF16* dStrchr(const UTF16* unistring, uint32 c)
{
   if(!unistring) return NULL;
   const UTF16* tmp = unistring;
   
   while ( *tmp  && *tmp != c)
      tmp++;

   return  (*tmp == c) ? tmp : NULL;
}

UTF16* dStrchr(UTF16* unistring, uint32 c)
{
   const UTF16* str = unistring;
   return const_cast<UTF16*>(dStrchr(str, c));
}

//-----------------------------------------------------------------------------
const UTF8* getNthCodepoint(const UTF8 *unistring, const uint32 n)
{
   const UTF8* ret = unistring;
   uint32 charsseen = 0;
   while( *ret && charsseen < n)
   {
      ret++;
      if((*ret & 0xC0) != 0x80)
         charsseen++;
   }
   
   return ret;
}

/* alternate utf-8 decode impl for speed, no error checking, 
   left here for your amusement:
   
   uint32 codeunit = codepoint + expectedByteCount - 1;
   uint32 i = 0;
   switch(expectedByteCount)
   {
      case 6: ret |= ( *(codeunit--) & 0x3f ); i++;            
      case 5: ret |= ( *(codeunit--) & 0x3f ) << (6 * i++);    
      case 4: ret |= ( *(codeunit--) & 0x3f ) << (6 * i++);    
      case 3: ret |= ( *(codeunit--) & 0x3f ) << (6 * i++);    
      case 2: ret |= ( *(codeunit--) & 0x3f ) << (6 * i++);    
      case 1: ret |= *(codeunit) & byteMask8LUT[expectedByteCount] << (6 * i);
   }
*/

//------------------------------------------------------------------------------
// Byte Order Mark functions

bool chompUTF8BOM( const char *inString, char **outStringPtr )
{
   *outStringPtr = const_cast<char *>( inString );

   uint8 bom[4];
   memcpy( bom, inString, 4 );

   bool valid = isValidUTF8BOM( bom );

   // bytes.
   if( valid )
      (*outStringPtr) += 3; // SEE ABOVE!! -pw

   return valid;
}

bool isValidUTF8BOM( uint8 bom[4] )
{
   // Is it a BOM?
   if( bom[0] == 0 )
   {
      // Could be UTF32BE
      if( bom[1] == 0 && bom[2] == 0xFE && bom[3] == 0xFF )
      {
         //Con::warnf( "Encountered a UTF32 BE BOM in this file; Torque does NOT support this file encoding. Use UTF8!" );
         return false;
      }

      return false;
   }
   else if( bom[0] == 0xFF )
   {
      // It's little endian, either UTF16 or UTF32
      if( bom[1] == 0xFE )
      {
         if( bom[2] == 0 && bom[3] == 0 )
		 {
			//Con::warnf( "Encountered a UTF32 LE BOM in this file; Torque does NOT support this file encoding. Use UTF8!" );
		 }            
         else
		 {
			//Con::warnf( "Encountered a UTF16 LE BOM in this file; Torque does NOT support this file encoding. Use UTF8!" );
		 }
          
      }

      return false;
   }
   else if( bom[0] == 0xFE && bom[1] == 0xFF )
   {
      //Con::warnf( "Encountered a UTF16 BE BOM in this file; Torque does NOT support this file encoding. Use UTF8!" );
      return false;
   }
   else if( bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF )
   {
      // Can enable this if you want -pw
      //Con::printf("Encountered a UTF8 BOM. Torque supports this.");
      return true;
   }

   // Don't print out an error message here, because it will try this with
   // every script. -pw
   return false;
}
#ifdef WINDOWS
bool isLeadbyte( const UTF8 c )
{
	return IsDBCSLeadByte(c) != 0;
}
#endif
