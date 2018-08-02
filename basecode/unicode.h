

#ifndef _UNICODE_H_
#define _UNICODE_H_

#include "basecode/BaseType.h"


UTF16* convertUTF8toUTF16( const UTF8 *unistring);
UTF8*  convertUTF16toUTF8( const UTF16 *unistring);

uint32 convertUTF8toUTF16(const UTF8 *unistring, UTF16 *outbuffer, uint32 len);
uint32 convertUTF16toUTF8( const UTF16 *unistring, UTF8  *outbuffer, uint32 len);

UTF32  oneUTF8toUTF32( const UTF8 *codepoint,  uint32 *unitsWalked = NULL);
UTF32  oneUTF16toUTF32(const UTF16 *codepoint, uint32 *unitsWalked = NULL);
UTF16  oneUTF32toUTF16(const UTF32 codepoint);
uint32    oneUTF32toUTF8( const UTF32 codepoint, UTF8 *threeByteCodeunitBuf);

uint32 dStrlen(const UTF16 *unistring);
uint32 dStrlen(const UTF32 *unistring);
uint32 dStrlen(const UTF8 *str);

uint32 dStrncmp(const UTF16* unistring1, const UTF16* unistring2, uint32 len);

UTF16* dStrrchr(UTF16* unistring, uint32 c);
const UTF16* dStrrchr(const UTF16* unistring, uint32 c);

UTF16* dStrchr(UTF16* unistring, uint32 c);
const UTF16* dStrchr(const UTF16* unistring, uint32 c);

const UTF8* getNthCodepoint(const UTF8 *unistring, const uint32 n);
bool chompUTF8BOM( const char *inString, char **outStringPtr );
bool isValidUTF8BOM( uint8 bom[4] );

UTF16* MByteToWChar(const UTF8* lpcszStr);
UTF8* WCharToMByte(const UTF16* lpcwszStr);

#ifdef WINDOWS
bool isLeadByte(const UTF8 c);
#endif

#endif // _UNICODE_H_