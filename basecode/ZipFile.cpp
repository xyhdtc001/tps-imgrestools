#include "assert.h"

#include "7z/Types.h"
#include "7z/lzma/Alloc.h"
#include "basecode/ZipFile.h"
#include "basecode/VerControl.h"
#include "basecode/BaseLog.h"

#ifdef USE_ZIP_ZLIB

extern "C"
{
	#include "3rd/zlib/zlib.h"
};

const int ZIP_BUFF_LEN = 10240;

bool ZipFile::zipFile( VFile& inFile, VFile& outFile )
{
#ifdef WINDOWS
	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	if ( Z_OK!=deflateInit(&strm, 9) )
		return false;

	uint8 inBuff[ZIP_BUFF_LEN];
	uint8 outBuff[ZIP_BUFF_LEN];
	
	int32 flush = Z_NO_FLUSH;
	PACK_FILE_HDR hdr;
	hdr.fileLen = inFile.getFileLen();
	if ( !outFile.writeData(&hdr,sizeof(hdr)) )
	{
		LogOut( RES, LOG_ERR, "write pack hdr err" );
		return false;
	}
	int32 iRead = 0;
	while( !inFile.isEof() )
	{
		iRead = inFile.read( inBuff, ZIP_BUFF_LEN );
		strm.next_in = inBuff;
		strm.avail_in = iRead;
		do
		{
			strm.next_out = outBuff;
			strm.avail_out = ZIP_BUFF_LEN;
			if ( inFile.isEof() )
				flush = Z_FINISH;
			else
				flush = Z_NO_FLUSH;
			if ( deflate(&strm,flush) == Z_STREAM_ERROR )
				return false;
			outFile.writeData( outBuff, ZIP_BUFF_LEN-strm.avail_out );
		}
		while( strm.avail_out==0 );
	}
	deflateEnd( &strm );
#endif
	return true;
}

ref_ptr<FileMemStream> ZipFile::unZipFileStream( ref_ptr<FileMemStream> fileStream )
{
#ifdef WINDOWS
	PACK_FILE_HDR hdr;
	fileStream->read( sizeof(hdr), &hdr );
	if ( hdr.fileLen > MAX_FILE_LEN )
	{
		LogOut( RES, LOG_ERR, "file len exceeded" );
		return NULL;
	}
	ref_ptr<FileMemStream> outStream = new ( hdr.fileLen ) FileMemStream( hdr.fileLen );

	z_stream strm;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	if ( Z_OK!=inflateInit(&strm) )
		return false;
	
	strm.avail_in = fileStream->getStreamSize()-sizeof(hdr);
	strm.next_in = (uint8*)fileStream->getPoint()+sizeof(hdr);
	strm.avail_out = outStream->getStreamSize();
	strm.next_out = (uint8*)outStream->getPoint();
	
	if ( inflate(&strm,Z_FINISH) == Z_STREAM_ERROR )
		return NULL;
	assert( strm.avail_out == 0 );
	inflateEnd( &strm );
	return outStream;
#else
    return NULL;
#endif
}

#else 

#ifdef USE_ZIP_LZMA

#include "7z/lzmaread.h"

typedef struct
{
	SRes (*read)(void *p, void *buf, size_t *size);
    /* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) < input(*size)) is allowed */

	VFile *file;
} ZIP_SeqInStream;


typedef struct
{
	size_t (*write)( void *p, const void *buf, size_t size );
	VFile *file;
} ZIP_SeqOutStream;


typedef struct
{
	size_t (*write)( void *p, const void *buf, size_t size );
	ref_ptr<MemStream> stream;
} ZIP_SeqOutStreamMem;


typedef struct
{
	SRes (*read)( void *p, void *buf, size_t *size );
	ref_ptr<MemStream> _strm;
} ZIP_SeqInStream_D;


typedef struct
{
	size_t (*write)(void *p, const void *buf, size_t size);
	ref_ptr<MemStream> _strm;
} ZIP_SeqOutStream_D;


SRes zipRead( void *p, void *buf, size_t *size )
{
	*size = ((ZIP_SeqInStream *)p)->file->read( (unsigned char *)buf, (uint32)*size );
	return SZ_OK;
}

size_t zipWrite( void *p, const void *buf, size_t size )
{
	return ((ZIP_SeqOutStream *)p)->file->write( (unsigned char *)buf, (uint32)size );
}

size_t zipWriteToMem( void *p, const void *buf, size_t size )
{
	if ( ((ZIP_SeqOutStreamMem*)p)->stream->write((uint32)size, buf) )
		return size;
	else
		return 0;
}

SRes zipRead_D(void *p, void *buf, size_t *size)
{
	*size = ((ZIP_SeqInStream_D *)p)->_strm->readEx( (uint32)*size, buf );
	return SZ_OK;
}

size_t zipWrite_D(void *p, const void *buf, size_t size)
{
	if ( ((ZIP_SeqOutStream_D *)p)->_strm->write((uint32)size, buf) )
		return size;
	else
		return 0;
}

bool ZipFile::zipFile( VFile& inFile, VFile& outFile )
{
	#ifdef WINDOWS
	ZIP_SeqInStream inStrm;
	ZIP_SeqOutStream outStrm;
	inStrm.file = &inFile;
	inStrm.read = zipRead;
	outStrm.file = &outFile;
	outStrm.write = zipWrite;
	return (SZ_OK == _ps_lzma_encode((ISeqOutStream *)(&outStrm), (ISeqInStream *)(&inStrm), inFile.getFileLen()));
	#else
    return false;
	#endif
}


ref_ptr<MemStream> ZipFile::zipFileToMem( VFile& inFile )
{
#ifdef WINDOWS
	uint32 len = inFile.getFileLen();
	ref_ptr<MemStream> pStream = new (len) MemStream( len );
	ZIP_SeqInStream inStrm;
	ZIP_SeqOutStreamMem outStrm;
	inStrm.file = &inFile;
	inStrm.read = zipRead;
	outStrm.stream = pStream;
	outStrm.write = zipWriteToMem;
	if ( SZ_OK == _ps_lzma_encode((ISeqOutStream *)(&outStrm),(ISeqInStream *)(&inStrm), len) )
		return pStream;
	else
		return NULL;
#else
	return false;
#endif
}

/*
ref_ptr<FileMemStream> ZipFile::unZipFileStream( ref_ptr<FileMemStream> fileStream )
{
	#ifdef WINDOWS
	ZIP_SeqInFileStream_D inStrm;
	ZIP_SeqOutFileStream_D outStrm;
	ref_ptr<FileMemStream> ret;
	uint32 unpackSize;
	int i;
	unsigned char header[LZMA_PROPS_SIZE + 8];
	size_t size = sizeof(header);
	fileStream->read( (uint32)size, &header );

	unpackSize = 0;
	for (i = 0; i < 8; i++)
		unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

	fileStream->setPosition(0);
	ret = new (unpackSize)FileMemStream(unpackSize);

	inStrm._strm = fileStream;
	inStrm.read = zipRead_D;

	outStrm._strm = ret;
	outStrm.write = zipWrite_D;
	SRes res = _ps_lzma_decode((ISeqOutStream *)(&outStrm), (ISeqInStream *)(&inStrm));
	if (res == SZ_OK)
	{
		return ret;
	}
	return NULL;
	#else

    return NULL;

	#endif
}
*/


ref_ptr<MemStream> ZipFile::unZipStream( ref_ptr<MemStream> memStream )
{
#ifdef WINDOWS
	ZIP_SeqInStream_D inStrm;
	ZIP_SeqOutStream_D outStrm;
	ref_ptr<MemStream> ret;
	uint32 unpackSize;
	int i;
	unsigned char header[LZMA_PROPS_SIZE + 8];
	size_t size = sizeof(header);
	memStream->read( (uint32)size, &header );

	unpackSize = 0;
	for ( i = 0; i < 8; i++ )
		unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

	//if ( unpackSize >= 16000000 )
	//	return NULL;
	if (unpackSize >= (100*1024*1024))
		return NULL;

	memStream->setReadPos( 0 );
	ret = new (unpackSize)MemStream( unpackSize );
	inStrm._strm = memStream;
	inStrm.read = zipRead_D;
	outStrm._strm = ret;
	outStrm.write = zipWrite_D;
	SRes res = _ps_lzma_decode( (ISeqOutStream *)(&outStrm), (ISeqInStream *)(&inStrm) );
	
	if ( res == SZ_OK )
	{
		return ret;
	}
	return NULL;
#else

	return NULL;

#endif
}


bool ZipFile::unZipFile( VFile& inFile, VFile& outFile )
{
#ifdef WINDOWS
	ZIP_SeqInStream inStrm;
	ZIP_SeqOutStream outStrm;
	inStrm.file = &inFile;
	inStrm.read = zipRead;
	outStrm.file = &outFile;
	outStrm.write = zipWrite;

	SRes res = _ps_lzma_decode((ISeqOutStream *)(&outStrm), (ISeqInStream *)(&inStrm));
	if (res == SZ_OK)
	{
		return true;
	}
	return false;
#else

	return false;

#endif
}

#endif

#endif
