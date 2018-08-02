#ifndef __LZMAREAD_H__
#define __LZMAREAD_H__

// #include "vfs/pconfig.h"
#include "7z/LZMA/LzmaEnc.h"
#include "7z/LZMA/LzmaDec.h"
// #include "vfs/pshell.h"
#include "7z/LZMA/Alloc.h"

// SRes ps_fwrite_lzma(uint8 *p, uint32 l, PS_FILE *file);
// SRes ps_fread_lzma(uint8 *p, uint32 l, PS_FILE *file);
// uint32 ps_lzma_get_unpack_size(PS_FILE *file);

/*----------------------------------*/

SRes _ps_lzma_encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize);
SRes _ps_lzma_decode(ISeqOutStream *outStream, ISeqInStream *inStream);
void *ps_lzma_alloc(void *p, size_t size);
void ps_lzma_free(void *p, void *address);
#endif