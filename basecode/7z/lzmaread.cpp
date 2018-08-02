#include <assert.h>
#include "7z/lzmaread.h"

void *ps_lzma_alloc(void *p, size_t size) { p = p; return MyAlloc(size); }
void ps_lzma_free(void *p, void *address) { p = p; MyFree(address); }
static ISzAlloc g_Alloc = { ps_lzma_alloc, ps_lzma_free };

#define IN_BUF_SIZE (1 << 16)
#define OUT_BUF_SIZE (1 << 16)

SRes _ps_lzma_decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
					UInt64 unpackSize)
{
	int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
	Byte inBuf[IN_BUF_SIZE];
	Byte outBuf[OUT_BUF_SIZE];
	size_t inPos = 0, inSize = 0, outPos = 0;
	LzmaDec_Init(state);
	for (;;)
	{
		if (inPos == inSize)
		{
			inSize = IN_BUF_SIZE;
			RINOK(inStream->Read(inStream, inBuf, &inSize));
			inPos = 0;
		}
		{
			SRes res;
			SizeT inProcessed = inSize - inPos;
			SizeT outProcessed = OUT_BUF_SIZE - outPos;
			ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
			ELzmaStatus status;
			if (thereIsSize && outProcessed > unpackSize)
			{
				outProcessed = (SizeT)unpackSize;
				finishMode = LZMA_FINISH_END;
			}

			res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
				inBuf + inPos, &inProcessed, finishMode, &status);
			inPos += inProcessed;
			outPos += outProcessed;
			unpackSize -= outProcessed;

			if (outStream)
				if (outStream->Write(outStream, outBuf, outPos) != outPos)
					return SZ_ERROR_WRITE;

			outPos = 0;

			if (res != SZ_OK || thereIsSize && unpackSize == 0)
				return res;

			if (inProcessed == 0 && outProcessed == 0)
			{
				if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
					return SZ_ERROR_DATA;
				return res;
			}
		}
	}
}


SRes _ps_lzma_decode( ISeqOutStream *outStream, ISeqInStream *inStream )
{
	UInt64 unpackSize;
	SRes res = 0;
	CLzmaDec state;

	/* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
	unsigned char header[LZMA_PROPS_SIZE + 8];

	/* Read and parse header */
	//RINOK(SeqInStream_Read(inStream, header, sizeof(header)));
	size_t size = sizeof( header );
	inStream->Read( inStream, header, &size );

	unpackSize = 0;
	for ( int i = 0; i < 8; i++ )
		unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << ( i * 8 );
	LzmaDec_Construct( &state );
	RINOK( LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc) );
	res = _ps_lzma_decode2( &state, outStream, inStream, unpackSize );
	LzmaDec_Free( &state, &g_Alloc );
	return res;
}


SRes _ps_lzma_encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize)
{
	CLzmaEncHandle enc;
	SRes res;
	CLzmaEncProps props;

	enc = LzmaEnc_Create(&g_Alloc);
	if (enc == 0)
		return SZ_ERROR_MEM;

	LzmaEncProps_Init(&props);
	res = LzmaEnc_SetProps(enc, &props);

	if (res == SZ_OK)
	{
		Byte header[LZMA_PROPS_SIZE + 8];
		size_t headerSize = LZMA_PROPS_SIZE;
		int i;

		res = LzmaEnc_WriteProperties(enc, header, &headerSize);
		for (i = 0; i < 8; i++)
			header[headerSize++] = (Byte)(fileSize >> (8 * i));
		if (outStream->Write(outStream, header, headerSize) != headerSize)
			res = SZ_ERROR_WRITE;
		else
		{
			if (res == SZ_OK)
				res = LzmaEnc_Encode(enc, outStream, inStream, NULL, &g_Alloc, &g_Alloc);
		}
	}
	LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
	return res;
}

// SRes ps_fwrite_lzma(uint8 *p, uint32 l, PS_FILE *file)
// {
// 	PS_SeqInStream inStrm;
// 	PS_SeqOutStream outStrm;
// 
// 	inStrm.buf = (char *)p;
// 	inStrm.len = l;
// 	inStrm.read = _ps_lzma_read;
// 	
// 	outStrm.file = file;
// 	outStrm.key = 0x66;
// 	outStrm.write = _ps_lzma_write;
// 
// #ifdef PVFS_DEBUG_NOZIP
// 	outStrm.write(&outStrm, p, l);
// 	return SZ_OK;
// #else
//  	return _ps_lzma_encode((ISeqOutStream *)(&outStrm), (ISeqInStream *)(&inStrm), l);
// #endif
// }

// SRes ps_fread_lzma(uint8 *p, uint32 l, PS_FILE *file)
// {
// 	PS_SeqInStream_D inStrm;
// 	PS_SeqOutStream_D outStrm;
// 
// 	pvfs_seek(0, PVFS_SEEK_SET, file->file);
// 	inStrm.file = file;
// 	inStrm.key = 0x66;
// 	inStrm.read = _ps_lzma_read_d;
// 
// 	outStrm.buf = (char *)p;
// 	outStrm.len = 0;
// 	outStrm.write = _ps_lzma_write_d;
// 
// #ifdef PVFS_DEBUG_NOZIP
// 	inStrm.read(&inStrm, p, &l);
// 	return SZ_OK;
// #else
// 	return _ps_lzma_decode((ISeqOutStream *)(&outStrm), (ISeqInStream *)(&inStrm));
// #endif
// }

// uint32 ps_lzma_get_unpack_size(PS_FILE *file)
// {
// #ifdef PVFS_DEBUG_NOZIP
// 	return file->file->size;
// #else
// 	PS_SeqInStream_D inStrm;
// 	UInt64 unpackSize;
// 	unsigned char header[LZMA_PROPS_SIZE + 8];
// 	size_t size = sizeof(header);
// 	unsigned int i;
// 
// 	inStrm.file = file;
// 	inStrm.key = 0x66;
// 	inStrm.read = _ps_lzma_read_d;
// 
// 	((ISeqInStream *)(&inStrm))->Read((void *)(&inStrm), header, &size);
// 
// 	unpackSize = 0;
// 	for (i = 0; i < 8; i++)
// 		unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);
// 
// 	assert(unpackSize < 0x00000000ffffffff);
// 	return (uint32)unpackSize;
// #endif
// }