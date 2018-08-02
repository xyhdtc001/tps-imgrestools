#include <sys/stat.h>

#if defined WINDOWS
#include <iostream>
#include "windows.h"
#if defined METRO
#else
#endif
#else
#include <unistd.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "basecode/BaseLog.h"
#include "basecode/StringData.h"
#include "basecode/VFile.h"

VFile::VFile() : _bOpen( false )
{

}


VFile::VFile( const Data& fullFilePath, int mode ) : _bOpen( false )
{
	openFile( fullFilePath, mode );
}

VFile::~VFile()
{
	if( _bOpen )
		fclose( hFile );
}


bool VFile::openFile( const Data& fullFilePath, int mode )
{
	if ( _bOpen )                                                                                                                                                      
	{
		fclose( hFile );
		_bOpen = false;
	}
	_path = fullFilePath;
	if ( mode & NONE )
	{
		return true;
	}
	if( mode & READ )
	{
		hFile = fopen( _path.c_str(), "rb" );
		if ( hFile == NULL )
		{
			if ( mode & CREATE )
			{
				hFile = fopen( _path.c_str(), "wb+" );
				fclose( hFile );
				hFile = fopen( _path.c_str(), "rb" );
			}
		}		
	}
	else if( mode&READWRITE )							//覆盖创建读写
	{
		hFile = fopen( _path.c_str(), "wb+" );
	}
	else if( mode&MODIFY )
	{
		hFile = fopen( _path.c_str(), "r+b" );
		if ( hFile==NULL && mode&CREATE )
		{
			hFile = fopen( _path.c_str(), "wb+" );
			fclose( hFile );
			hFile = fopen( _path.c_str(), "r+b" );
		}
	}
	else if( mode&READWRITE && mode&CREATE ) //必须创建
	{
		if ( NULL!=(hFile=fopen(_path.c_str(),"rb")) )
		{
			fclose( hFile );
			LogOut( MAIN, LOG_ERR, "file %s exists", _path.c_str() );
			return false;
		}
		hFile = fopen( _path.c_str(), "wb+" );
	}
	else if( mode & APPEND )	//添加
	{
		hFile = fopen( _path.c_str(), "ab+" );
	}
	else
	{
		LogOut( MAIN, LOG_ERR, "File open mode error" );
		return false;
	}
	if ( hFile == 0 )
	{
		//LogOut( MAIN, LOG_ERR, "File %s open error", _path.c_str() );
		return false;
	}
	else
	{
		_bOpen = true;
	}
	return true;
}


bool VFile::seekTo( int to, int orign )
{
	if ( fseek(hFile,to,orign) == 0 )
		return true;
	else
		return false;
}


int VFile::read( unsigned char *buf, int len )
{
	int iLeft = len;
	while ( iLeft > 0 )
	{
		if ( feof(hFile) || ferror(hFile) )
			return len-iLeft;
		iLeft -= (int)fread( buf+(len-iLeft), 1, iLeft, hFile );
	}
	return len;
}

bool VFile::readData( unsigned char* buf, int len )
{
	return ( read(buf,len) == len );
}

int VFile::readLine( char *buff, int maxlen )
{
	char ch;
	for ( int i=0; i<maxlen; i++ )
	{
		if ( fread(&ch,1,1,hFile) == 0 )
		{
			buff[i] = '\0';
			if ( i > 0 )
				return i;
			else
				return -1;
		}
		if ( ch=='\n' )
		{
			if ( i>0 && buff[i-1]=='\r' )
			{
				buff[i-1] = '\0';
				return i-1;
			}
			else
			{
				buff[i] = '\0';
				return  i;
			}
		}
		else
		{
			buff[i] = ch;
		}
	}
	return maxlen;
}

int VFile::readLine( Data& dtLine, int maxlen )
{
	char tmp[8192];
	memset( tmp, 0, 8192 );
	int iRet = readLine( tmp, maxlen );
	dtLine = tmp;
	return iRet;
}
	
int VFile::write( const unsigned char *buff, uint32 len )
{
	return (int)fwrite( buff, 1, len, hFile );
}

int VFile::write( uint32 len, const unsigned char* buff )
{
	return write( buff, len );
}

int VFile::write( const Data& data )
{
	const char* pData = data.c_str();
	int len = data.length();
	int iWrited = 0;
	while ( iWrited < len )
	{
		int iRet = write( (unsigned char*)(pData+iWrited), len-iWrited );
		if ( iRet <= 0 )
			break;
		iWrited += iRet;
	}
	return iWrited;
}

int VFile::writeLine( const Data& date )
{
	return write( date + CRLF );
}

void VFile::flush()
{
	fflush( hFile );
}

Data VFile::getFilePath() const
{
	return _path;
}

Data VFile::getExtName() const
{
	int pos = 0;
	if ( (pos=_path.findlast(".")) == -1 )
		return "";
	else
		return _path.substr( pos+1 );
}


bool VFile::isFileExist( const Data& fullFilePath )
{
#ifdef WINDOWS
	bool bExits;
	FILE* fp = fopen( fullFilePath.c_str(), "rb" );
	if ( fp == NULL )
	{
		bExits = false;
	}
	else
	{
		bExits = true;
		fclose( fp );
	}
	return bExits;
#else
	struct stat sys_stat;
	return (stat(fullFilePath.c_str(), &sys_stat) == 0);
	//return access(fullFilePath.getData(),F_OK) == 0;
#endif

}


bool VFile::deleteFile( const Data& fullFilePath )
{
#ifdef WINDOWS
#ifdef METRO
	return false;
#else
	if ( DeleteFile(fullFilePath.c_str()) )
		return true;
	else
		return false;
#endif
#else
	if ( remove(fullFilePath.c_str()) == 0 )
		return true;
	else
		return false;
#endif	
}


bool VFile::move( const Data& from, const Data& to, bool replace )
{
#if defined WINDOWS
#ifdef METRO
	return false;
#else
	if ( replace )
		return ::MoveFileEx(from.c_str(),to.c_str(),MOVEFILE_REPLACE_EXISTING)==TRUE;
	else
		return ::MoveFileEx(from.c_str(), to.c_str(),MOVEFILE_REPLACE_EXISTING)==TRUE;
#endif
#else
	return ::rename( from.c_str(), to.c_str() );
#endif
}


bool VFile::rename( const Data& path, const Data& from, const Data& to )
{
#if defined WINDOWS
#ifdef METRO
	return false;
#else
	SHFILEOPSTRUCT fileOp;
	memset( &fileOp, 0, sizeof(SHFILEOPSTRUCT) );
	char fromDir[255] = {0,};
	char toDir[255] = {0,};
	Data dtPath = path;
	dtPath.makePath();
	dtPath.formatPath( false );
	Data dtFromPath = dtPath + from;
	Data dtToPath = dtPath + to;
	strcpy( fromDir, dtFromPath.c_str() );
	strcpy( toDir, dtToPath.c_str() );
	fileOp.pFrom = fromDir;
	fileOp.pTo = toDir;
	fileOp.fFlags = FOF_NOCONFIRMATION;
	fileOp.wFunc = FO_RENAME;
	return (SHFileOperation(&fileOp)==0);
#endif
#else
	return false;
#endif
}



uint32 VFile::getFileLen( const Data &file )
{
#ifdef WINDOWS
#ifdef METRO
	return 0;
#else
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind;
	hFind = FindFirstFile( file.c_str(), &fileInfo );
	
	if ( hFind == INVALID_HANDLE_VALUE )
	{
		FindClose( hFind );
		return -1;
	}
	else
	{
		FindClose(hFind);
		return fileInfo.nFileSizeLow;
	}
#endif
#else
	struct stat statbuf;
	if (stat(file.c_str(), &statbuf) == -1)
	{
		return -1;
	}
	else
	{
		return statbuf.st_size;
	}
#endif
}



bool VFile::chmode( const Data& file, const Data& mod )
{
#if defined WINDOWS
	return false;
#else
#if defined METRO
	return false;
#else
	if ( chmod(file.c_str(), ((mod[(unsigned int)0]-'0')<<6) +((mod[(unsigned int)1]-'0')<<3)+(mod[(unsigned int)2]-'0')) == 0 )
		return true;
    return  false;
#endif
#endif
}




uint32 VFile::getFileLen()
{
	uint32 pos = getFilePos();
	seekTo( 0, VFile::VFILE_END );
	uint32 fileLen = getFilePos();
	seekTo( pos, VFILE_BEGIN );
	return fileLen;
}


uint32 VFile::getFileLeftLen()
{
	uint32 pos = getFilePos();
	seekTo( 0, VFile::VFILE_END );
	uint32 fileLen = getFilePos();
	seekTo( pos, VFILE_BEGIN );
	return fileLen-pos;
}


long VFile::getFilePos()
{
	return ftell( hFile );
}


void VFile::closeFile()
{
	if ( _bOpen )
	{
		fclose( hFile );
		_bOpen = false;
	}
}

bool VFile::copyFile( const Data& from, const Data& to, bool replace )
{
	if ( replace )
	{
		VFile::deleteFile( to );
	}
	else
	{
		if ( isFileExist(to) )
			return false;
	}
	VFile file;
	if ( !file.openFile(from,READ) )
		return false;
	if ( !file.copyTo(to) )
		return false;
	/*
	#if !defined WINDOWS
	struct stat tstat;
	stat( from.c_str(), &tstat );
	chmod( to.c_str(), tstat.st_mode );
	#endif
	*/
	return true;
}

bool VFile::copyTo( const Data& target )
{
	VFile tarFile;
	if ( !tarFile.openFile(target,READWRITE) )
		return false;
	unsigned char buff[2048];
	int iRead;
	while( ( iRead=read( buff, 2048 ) ) > 0 )
	{
		tarFile.write( buff, iRead );
	}
	tarFile.closeFile();
	/*
	#if !defined WINDOWS
	struct stat st;
	stat( getFilePath().c_str(), &st );
	chmod( target.c_str(), st.st_mode );
	#endif
	*/
	return true;
}


ref_ptr<MemStream>
VFile::getAllData()
{
	long fileLen = getFileLen();
	if ( fileLen <= 0 )
		return NULL;
	ref_ptr<MemStream> memStream = new ( fileLen ) MemStream( fileLen );
	uint8* pMem = (uint8*)memStream->getPoint();
	seekTo( 0, VFILE_BEGIN );

	int iReaded = 0,iRead;
	while ( iReaded < fileLen )
	{
		if ( (iRead=read(pMem+iReaded, fileLen-iReaded)) > 0 )
		{
			iReaded += iRead;
		}
		else
		{
			return NULL;
		}
	}
	return memStream;
}



ref_ptr<MemStream> VFile::getLeftData()
{
	uint32 filePos =getFilePos();
	uint32 fileLeftLen = getFileLeftLen();

	ref_ptr<MemStream> memStream = new( fileLeftLen ) MemStream( fileLeftLen );
	uint8* pMem = (uint8*)memStream->getPoint();
	uint32 iReaded=0,iRead;
	while ( iReaded < fileLeftLen )
	{
		if ( (iRead=read(pMem+iReaded, fileLeftLen-iReaded)) > 0 )
		{
			iReaded += iRead;
		}
		else
		{
			return NULL;
		}
	}
	seekTo( filePos, VFile::VFILE_BEGIN );
	return memStream;
}




bool
VFile::getAllData( uint8* data )
{
	int iRead = 0;
	long fileLen = getFileLen();
	if ( fileLen <= 0 )
		return NULL;
	seekTo( 0, VFILE_BEGIN );
	while ( iRead < fileLen )
	{
		int iReaded = 0;
		if ( (iReaded=read(data+iRead, fileLen-iRead)) > 0 )
		{
			iRead += iReaded;
		}
		else
		{
			return false;
		}
	}
	return true;
}


bool
VFile::getTime( const Data& path, uint64& create, uint64& access, uint64& write )
{
#ifdef WINDOWS
#ifdef METRO
	return 0;
#else
	HFILE fp = _lopen( path.c_str(), OF_READ );
	if ( fp == NULL )
		return false;
	uint64 val = 0;
	FILETIME createTime;
	FILETIME accessTime;
	FILETIME writeTime;
	if ( GetFileTime((HANDLE)fp,&createTime,&accessTime,&writeTime) )
	{
		create = (uint64)( ((PULARGE_INTEGER)(&createTime))->QuadPart / 10000000 );
		access = (uint64)( ((PULARGE_INTEGER)(&accessTime))->QuadPart / 10000000 );
		write = (uint64)( ((PULARGE_INTEGER)(&writeTime))->QuadPart / 10000000 );
		_lclose( fp );
		return true;
	}
	else
	{
		_lclose( fp );
		return false;
	}
	
#endif
#else
	return 0;
#endif
}


bool
VFile::resetAccessTime( const Data& path )
{
#ifdef WINDOWS
#ifdef METRO
	return false;
#else
	FILETIME ft;
	SYSTEMTIME st;
	GetSystemTime( &st );
	SystemTimeToFileTime( &st, &ft );
	HFILE fp = _lopen( path.c_str(), OF_WRITE );
	if ( fp == NULL )
		return false;
	if ( SetFileTime((HANDLE)fp,NULL,&ft,NULL) )
	{
		_lclose( fp );
		return true;
	}
	else
	{
		_lclose( fp );
		return false;
	}
#endif
#else
	return false;
#endif
}


bool VFile::writeData( const void* buff, uint32 len )
{
	uint8* pBuff = (uint8*)buff;
	uint32 writeLen = 0;
	while( len > 0 )
	{
		if ( (writeLen=(uint32)fwrite(buff,1,len,hFile)) <= 0 )
			return false;
		pBuff += writeLen;
		len -= writeLen;
	}
	return true;
}
