#include "basecode/directory.h"
#include "basecode/BaseLog.h"

#if defined WINDOWS
#if defined METRO
#else
#include <io.h>
#include <direct.h>
#include <windows.h>
#endif
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

VDirectory::VDirectory()
{
	_bIncludeFile = false;
	_bExcludeFile = false;
	_bExcludeDir = false;
}


VDirectory::VDirectory( const Data& dir, bool create )
{
	_bIncludeFile = false;
	_bExcludeFile = false;
	_bExcludeDir = false;
	open( dir, create );
}


Data VDirectory::getPath()
{
	return _directory;
}


bool
VDirectory::isDirectoryExist( const Data& dir  )
{	
#if defined WINDOWS
#if defined METRO
	WIN32_FIND_DATAW findData;
	memset( &findData, 0, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFileEx( (dir+"\\*").c_str(), &findData );
	if( INVALID_HANDLE_VALUE == hFind )
		return false;
	else
		FindClose( hFind );
		return true;
#else
	WIN32_FIND_DATA findData;
	memset( &findData, 0, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFile( (dir+"\\*").c_str(), &findData );
	if( INVALID_HANDLE_VALUE == hFind )
	{
		return false;
	}
	else
	{
		FindClose( hFind );
		return true;
	}
#endif
#else
	struct stat statbuf;
	if ( stat(dir.c_str(),&statbuf) == -1 )
		return false;
	if ( S_ISDIR(statbuf.st_mode) )
		return true;
	else
		return false;
#endif
}

bool VDirectory::createDir( const Data& dir )
{
	int pos = 0;
	int findPos = 0;
	while ( true )
	{
		findPos = pos;
		if ( (findPos=dir.find("/",pos)) == -1 )
		{
			if ( (findPos=dir.find("\\",pos)) == -1 )
				findPos = dir.length();
		}
		pos = findPos;
		Data dtDir = dir.substr( 0,pos );
		pos++;
		if ( !dtDir.isEmpty() )
		{
			if ( !isDirectoryExist(dtDir) )
			{
				#if defined WINDOWS
				#if defined METRO
				#else
				if ( !CreateDirectory(dtDir.c_str(),NULL) )
				{
					LogOut( MAIN, LOG_ERR, "Create directory error: %s", dir.c_str() );
					return false;
				}
				#endif
				#else
				if ( -1 == mkdir(dtDir.c_str(),S_IRWXU|S_IRWXG|S_IRWXO) )
				{
					log_out( MAIN, LOG_ERR, "Create directory error: %s", dir.c_str() );
					return false;
				}
				#endif
			}
		}
		if ( pos >= (int)dir.length() )
			break;
	}	
	return true;	
}

bool VDirectory::createFileDir( const Data& dir )
{
	int pos = 0;
	while ( true )
	{
		int iPos = 0;
		if ( (iPos=dir.find("/",pos)) == -1 )
		{
			if ( (iPos=dir.find("\\",pos)) == -1 )
				break;
		}
		pos = iPos;
		Data dtDir = dir.substr( 0,pos );
		pos++;
		if ( !dtDir.isEmpty() )
		{
			if ( !isDirectoryExist(dtDir) )
			{
				#if defined WINDOWS
				#if defined METRO
				#else
				if ( !CreateDirectory(dtDir.c_str(),NULL) )
				{
					LogOut( MAIN, LOG_ERR, "Create directory error: %s", dir.c_str() );
					return false;
				}
				#endif
				#else
				if ( -1 == mkdir(dtDir.c_str(),S_IRWXU|S_IRWXG|S_IRWXO) )
				{
					log_out( MAIN, LOG_ERR, "Create directory error: %s", dir.c_str() );
					return false;
				}
				#endif
			}
		}
	}	
	return true;
}


bool VDirectory::chDir( const Data& dir )
{
#if defined WINDOWS
#if defined METRO
	return false;
#else
	if ( SetCurrentDirectory(dir.c_str()) == TRUE )
		return true;
	else
		return false;
#endif
#else
	if ( chdir(dir.c_str()) == 0 )
		return true;
	else
		return false;
#endif
}

struct CopyParam
{
	Data dtTarDir;
};

bool CopyFileFunc( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	CopyParam* pCopyParam = (CopyParam*)pParam;
	Data dtTarFile = pCopyParam->dtTarDir + relaDir + filename;
	VFile file;
	if ( !file.openFile(filename, VFile::READ) )
		return false;
	return file.copyTo( dtTarFile );
}

bool CreateDirFunc( const Data& folderPath, const Data& folder, void* pParam )
{
	CopyParam* pCopyParam = (CopyParam*)pParam;
	return VDirectory::createDir( pCopyParam->dtTarDir+folderPath );
}

bool VDirectory::copyDir( const Data& srcDir, const Data& tarDir )
{
	Data dtSrcDir = srcDir;
	dtSrcDir.makePath();
	Data dtTarDir = tarDir;
	dtTarDir.makePath();

	CopyParam param;
	param.dtTarDir = dtTarDir;
	createDir( tarDir );
	return EnumAllFunc( dtSrcDir, "", CopyFileFunc, CreateDirFunc, &param );
}


bool VDirectory::isDirEmpty( const Data& fullDir )
{
#if defined WINDOWS
#if defined METRO
#else
#endif
#else
	Data dtDir = fullDir;
	dtDir.makePath();
	DIR* dir;
	if ( (dir=opendir(dtDir.c_str())) == NULL )
		return false;
	struct dirent *pDirent;
	while( ( pDirent = readdir(dir) ) != NULL  )
	{
		struct stat statbuf;
		Data dtPath = dtDir;
		dtPath += pDirent->d_name;
		if ( stat(dtPath.c_str(),&statbuf) == -1 )
			continue;
		if ( S_ISDIR(statbuf.st_mode) )
		{
			if ( pDirent->d_name && strcmp(pDirent->d_name,".") && strcmp(pDirent->d_name,"..") )
			{
				if ( !isDirEmpty(dtPath) )
				{
					closedir( dir );
					return false;
				}
			}
		}
		else
		{
			closedir( dir );
			return false;
		}
	}
	closedir( dir );
#endif
	return true;
}

bool VDirectory::open( const Data& directory, bool create )
{
	if ( directory.isEmpty() )
	{
		log_out(MAIN, LOG_ERR, "Directory is empty" );
		return false;
	}

	Data dir = directory;
	int pos;
	Data dtData = "\\";
	while ( (pos=dir.find(dtData)) != -1 )
    {
        dir.replace( pos, 1, "/" );
    }
	_directory.clear();

	bool bDirExist = isDirectoryExist( dir );
	if ( !bDirExist )
	{
		if ( create )
		{
			if ( !createDir(dir) )
				return false;
		}
		else
		{
			LogOut( MAIN, LOG_ERR, "Open directory error: %s", dir.c_str() );
			return false;
		}
	}
	_directory = dir;
	_directory.makePath();
	return true;
}


bool VDirectory::rename( const Data& from, const Data& to )
{
#if defined WINDOWS
#ifdef METRO
	return false;
#else
	if ( MoveFileEx(from.c_str(),to.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED|MOVEFILE_WRITE_THROUGH)  )
		return true;
	else
		return true;
#endif
#else
	if ( ::rename(from.c_str(),to.c_str()) == 0 )
		return true;
	else
		return false;
#endif
}


bool VDirectory::EnumAllFunc( const Data& fullDir, const Data& relaDir, EnumFunc func, FolderFunc folderFunc, void* pParam, bool bChildDir)
{
#ifdef WINDOWS
#ifdef METRO
	return false;
#else
	WIN32_FIND_DATA findData;
	if( fullDir.isEmpty() )
	{
		LogOut( MAIN, LOG_ERR, "open directory first" );
		return false;
	}
	memset( &findData, 0, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFile( (fullDir+"\\*").c_str(), &findData );
	FindNextFile( hFind, &findData );
	while( FindNextFile( hFind, &findData ) )
	{
		if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			Data dtPath = fullDir + findData.cFileName + "\\";
			Data dtRelaPath = relaDir + findData.cFileName + "\\";
			if (bChildDir && !EnumAllFunc(dtPath,dtRelaPath,func,folderFunc,pParam) )
			{
				FindClose( hFind );
				return false;
			}
				
			if ( folderFunc != NULL )
			{
				if ( !folderFunc(fullDir,findData.cFileName,pParam) )
				{
					FindClose( hFind );
					return false;
				}
			}
		}
		else if( findData.cFileName )
		{
			//if ( func(findData.cFileName,fullDir+findData.cFileName,"",pParam) )
			if (func && !func(findData.cFileName,fullDir,relaDir,pParam) )
			{
				break;
			}
		}
	}
	FindClose( hFind );
	return true;
#endif
#else
	DIR* dir;
	if ( (dir=opendir(fullDir.c_str())) == NULL )
		return false;
	struct dirent *pDirent;
	while( ( pDirent = readdir(dir) ) != NULL  )
	{
		struct stat statbuf;
		Data dtPath = fullDir;
		Data dtFile = pDirent->d_name;
		dtPath += dtFile;
		if ( stat(dtPath.c_str(),&statbuf) == -1 )
			continue;
		if ( S_ISDIR(statbuf.st_mode) )
		{
			if ( strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0 )
			{
				Data dtRelaPath = relaDir;
				dtRelaPath += pDirent->d_name;
				if ( folderFunc != NULL )
					folderFunc( dtPath, dtRelaPath, pParam );
				if ( !EnumAllFunc(dtPath+"/",dtRelaPath+"/",func,folderFunc,pParam) )
					return false;
			}
		}
		else
		{
			if ( !func(dtFile,dtPath,relaDir,pParam) )
				return false;
		}
	}
	closedir( dir );
	return true;
#endif
}


bool VDirectory::EnumDirs( FolderFunc func, void* pParam )
{
#ifdef WINDOWS
#ifdef METRO
#else
	WIN32_FIND_DATA findData;
	if( _directory.isEmpty() )
	{
		LogOut( MAIN, LOG_ERR, "open directory first" );
		return false;
	}
	memset( &findData, 0, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFile( (_directory+"\\*").c_str(), &findData );
	FindNextFile( hFind, &findData );
	bool bFinish = false;
	while( FindNextFile( hFind, &findData ) )
	{
		if( findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY && findData.cFileName )
		{
			Data dtDir = findData.cFileName;
			if ( _bExcludeDir )
			{
				if ( isExcludeFile(dtDir) )
					continue;
			}
			if ( !func(dtDir,dtDir,pParam) )
				return false;
		}
	}
	FindClose( hFind );
#endif
#else
	DIR* dir;
	if ( (dir=opendir(_directory.c_str())) == NULL )
		return false;
	struct dirent *pDirent;
	while( ( pDirent = readdir(dir) ) != NULL  )
	{
		struct stat statbuf;
		Data dtPath = _directory;
		dtPath += pDirent->d_name;
		if ( stat(dtPath.c_str(),&statbuf) == -1 )
			continue;
		if (S_ISDIR(statbuf.st_mode) )		
		{
			if ( strcmp(pDirent->d_name,".")!=0 && strcmp(pDirent->d_name,"..")!=0 )
			{
				Data dtDir = pDirent->d_name;
				if ( _bExcludeDir )
				{
					if ( isExcludeFile(dtDir) )
						continue;
				}
				if ( !func(dtDir, dtDir, pParam) )
					return false;
			}
		}
	}
	closedir( dir );
#endif
	return true;
}



bool VDirectory::EnumFiles( EnumFunc func, void* pParam )
{
	bool bRet = true;
#ifdef WINDOWS
#ifdef METRO
	return false;
#else
	WIN32_FIND_DATA findData;
	if ( _directory.isEmpty() )
	{
		LogOut( MAIN, LOG_ERR, "open directory first" );
		return false;
	}
	memset( &findData, 0, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFile( (_directory+"\\*").c_str(), &findData );
	FindNextFile( hFind, &findData );
	bool bFinish = false;
	while ( FindNextFile( hFind, &findData ) )
	{
		if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;
		if( findData.cFileName )
		{
			if ( _bIncludeFile )
			{
				if ( !isIncludeFile(findData.cFileName) )
					continue;
			}
			if ( _bExcludeFile )
			{
				if ( isExcludeFile(findData.cFileName) )
					continue;
			}
			Data dtFile = findData.cFileName;
			Data dtFilePath = _directory;
			dtFilePath.makePath();
			dtFilePath += dtFile;
			if ( !func(dtFile,dtFilePath,"",pParam) )
			{
				bRet = false;
				break;
			}
		}
	}
	FindClose( hFind );
#endif
#else
	DIR* dir;
	if ( (dir=opendir(_directory.c_str())) == NULL )
	{
		log_out( MAIN, LOG_ERR, "open directory err" );
		return false;
	}
	struct dirent *pDirent;
	while( ( pDirent = readdir(dir) ) != NULL  )
	{
		struct stat statbuf;
		Data dtFile = pDirent->d_name;
		Data dtPath = _directory;
		dtPath += dtFile;
		if ( stat(dtPath.c_str(),&statbuf) == -1 )
			continue;
		if ( !S_ISDIR(statbuf.st_mode) )
		{
			if ( _bIncludeFile )
			{
				if ( !isIncludeFile(pDirent->d_name) )
					continue;
			}
			if ( _bExcludeFile )
			{
				if ( isExcludeFile(pDirent->d_name) )
					continue;
			}
			if ( !func(dtFile,dtPath,"",pParam) )
			{
				bRet = false;
				break;
			}
		}
	}
	closedir( dir );
#endif
	return bRet;
}

//获取文件set
bool InsertVec( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	std::vector<Data>* pSetFile = (std::vector<Data>*)pParam;
	pSetFile->push_back( filename );
	return true;
}

void VDirectory::getFilesVec( std::vector<Data>* vecData )
{
	EnumFiles( InsertVec, vecData );
}


//获取文件列表
bool InsertFileList( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	VDirectory* pDir = (VDirectory*)pParam;
	pDir->_fileVec.push_back( filename );
	return true;
}

void VDirectory::getFiles()
{
	_fileVec.clear();
	if ( _directory.isEmpty() )
	{
		log_out( MAIN, LOG_ERR, "open directory first" );
		return;
	}
	EnumFiles( InsertFileList, this );
	/*
#if defined WINDOWS
	WIN32_FIND_DATA findData;
	
	memset( &findData, 0, sizeof( WIN32_FIND_DATA ) );
	HANDLE hFind = FindFirstFile( _directory+"\\*", &findData );
	FindNextFile( hFind, &findData );
	bool bFinish = false;
	while( FindNextFile( hFind, &findData ) )
	{
		if ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			continue;
		if ( findData.cFileName )
		{
			if ( _bIncludeFile )
			{
				if ( !isIncludeFile(findData.cFileName) )
					continue;
			}
			if ( _bExcludeFile )
			{
				if ( isExcludeFile(findData.cFileName) )
					continue;
			}
			_fileVec.push_back( findData.cFileName );
		}
	}
	FindClose( hFind );
#else
	DIR* dir;
	if ( (dir=opendir(_directory.getData())) == NULL )
		return;
	struct dirent *pDirent;
	while( ( pDirent = readdir(dir) ) != NULL  )
	{
		struct stat statbuf;
		Data dtPath = _directory;
		dtPath += pDirent->d_name;
		if ( stat(dtPath.getData(),&statbuf) == -1 )
			continue;
		if ( !S_ISDIR(statbuf.st_mode) )
		{
			if ( _bIncludeFile )
			{
				if ( !isIncludeFile(pDirent->d_name) )
					continue;
			}
			if ( _bExcludeFile )
			{
				if ( isExcludeFile(pDirent->d_name) )
					continue;
			}
			_fileVec.push_back( pDirent->d_name );
		}
	}
	closedir( dir );
#endif
	*/
}


bool InsertDirSet( const Data& folderPath, const Data& folder, void* pParam )
{
	std::set<Data>* pSet = (std::set<Data>*)pParam;
	pSet->insert( folder );
	return true;
}
void VDirectory::getSubDirsSet( std::set<Data>* pSet )
{
	EnumDirs( InsertDirSet, pSet );
}



bool InsertDirList( const Data& folderPath, const Data& folder, void* pParam )
{
	VDirectory* pDir = (VDirectory*)pParam;
	pDir->_dirVec.push_back( folder );
	return true;
}
void VDirectory::getSubDirs()
{
	_dirVec.clear();
	EnumDirs( InsertDirList,this );
}



Data VDirectory::getFileName( unsigned int index )
{
	if ( _fileVec.size() <= index )
		return "";
	return _fileVec[index];
}

Data VDirectory::getDirName( unsigned int index )
{
	if ( _dirVec.size() <= index )
		return "";
	return _dirVec[index];
}

uint32 VDirectory::getDirNum()
{
	return (uint32)_dirVec.size();
}

uint32 VDirectory::getFileNum()
{
	return (uint32)_fileVec.size();
}

bool VDirectory::removeAll( const Data& path, bool recursive )
{
	Data dtDir = path;
	int pos = 0;
	while ( (pos=dtDir.find("/")) != -1 )
	{
		dtDir.replace( pos, 1, "\\" );
	}
	dtDir.makePath();
#if defined WINDOWS
#ifdef METRO
	return false;
#else
	SHFILEOPSTRUCT FileOp; 
	FileOp.fFlags = FOF_NOCONFIRMATION|FOF_SILENT; 
	FileOp.hNameMappings = NULL; 
	FileOp.hwnd = NULL; 
	FileOp.lpszProgressTitle = NULL; 
	Data dtFiles = dtDir + "*.*";
	char buff[256] = {0,};
	strcpy( buff, dtFiles.c_str() );
	FileOp.pFrom = buff; 
	FileOp.pTo = NULL; 
	FileOp.wFunc = FO_DELETE; 
	return SHFileOperation(&FileOp) == 0;
#endif
#else
	DIR* dir;
	if ( (dir=opendir(path.c_str())) == NULL )
		return false;
	struct dirent *pDirent;
	Data dtPath = path;
	dtPath.makePath();
	while( ( pDirent=readdir(dir) ) != NULL  )
	{
		if ( strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0 )
			continue;
		Data dtVal = dtPath + Data(pDirent->d_name);
		struct stat statbuf;
		if ( stat(dtVal.c_str(),&statbuf) == -1 )
		{
			closedir( dir );
			return false;
		}
		if ( S_ISDIR(statbuf.st_mode) )
		{
			if ( recursive )
			{
				if ( !removeAll( dtVal ) )
				{
					closedir( dir );
					return false;
				}
				if ( remove(dtVal.c_str()) != 0 )
				{
					closedir( dir );
					return false;
				}
			}
		}
		else
		{
			if ( remove(dtVal.c_str()) != 0 )
			{
				closedir( dir );
				return false;
			}
		}
	}
	closedir( dir );
	return true;
#endif
}


bool VDirectory::removeAllBySubfix( const Data& path, const Data& subfix, bool recursive )
{
#ifdef WINDOWS
#ifdef METRO
#else
#endif
#else
	DIR* dir;
	if ( (dir=opendir(path.c_str())) == NULL )
		return false;
	struct dirent *pDirent;
	Data dtPath = path;
	dtPath.makePath();
	while( ( pDirent=readdir(dir) ) != NULL  )
	{
		if ( strcmp(pDirent->d_name,".")==0 || strcmp(pDirent->d_name,"..")==0 )
			continue;
		Data dtFile = pDirent->d_name;
		Data dtVal = dtPath + dtFile;
		struct stat statbuf;
		if ( stat(dtVal.c_str(),&statbuf) == -1 )
		{
			closedir( dir );
			return false;
		}
		if ( S_ISDIR(statbuf.st_mode) )
		{
			if ( recursive )
			{
				if ( !removeAllBySubfix( dtVal, subfix ) )
				{
					closedir( dir );
					return false;
				}
			}
		}
		else
		{
			if ( dtFile.isEndWith(subfix) )
			{
				if ( remove(dtVal.c_str()) != 0 )
				{
					closedir( dir );
					return false;
				}
			}
		}
	}
	closedir( dir );
	return true;
#endif
	return true;
}


bool VDirectory::removeDir( const Data& dir )
{
	Data dtDir = dir;
	int pos = 0;
	while ( (pos=dtDir.find("/")) != -1 )
		dtDir.replace( pos, 1, "\\" );
#ifdef WINDOWS
#ifdef METRO
	return false;
#else
	SHFILEOPSTRUCT opStru;
	memset( &opStru, 0, sizeof(SHFILEOPSTRUCT) );
	opStru.fFlags = FOF_NOCONFIRMATION|FOF_SILENT; 
	char buff[256] = {0,};
	strcpy( buff, dtDir.c_str() );
	opStru.pFrom = buff;
	opStru.wFunc = FO_DELETE;
	if ( SHFileOperation(&opStru) == 0 )
		return true;
	else
		return false;
#endif
#else
	if ( removeAll( dir ) )
	{
		if ( remove(dir.c_str()) )
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
#endif
}


void VDirectory::setExcludeDir( const Data& data )
{
	_vecExcludeDir.clear();
	_bExcludeDir = true;
	std::vector<Data> vecData;
	data.split( "|", vecData );
	for ( std::vector<Data>::iterator iter=vecData.begin(); iter!=vecData.end(); iter++ )
	{
		_vecExcludeDir.push_back( *iter );
	}
}


void VDirectory::setIncludeFile( const Data& data )
{
	_vecIncludeFile.clear();
	_bIncludeFile = true;
	std::vector<Data> vecData;
	data.split( "|", vecData );
	for ( std::vector<Data>::iterator iter=vecData.begin(); iter!=vecData.end(); iter++ )
	{
		_vecIncludeFile.push_back( *iter );
	}
}


void VDirectory::setExcludeFile( const Data& data )
{
	_vecExcludeFile.clear();
	_bExcludeFile = true;
	std::vector<Data> vecData;
	data.split( "|", vecData );
	for ( std::vector<Data>::iterator iter=vecData.begin(); iter!=vecData.end(); iter++ )
	{
		_vecExcludeFile.push_back( *iter );
	}
}


bool VDirectory::isIncludeFile( const Data& file )
{
	for ( std::vector<Data>::iterator iter=_vecIncludeFile.begin(); iter!=_vecIncludeFile.end(); iter++ )
	{
		if ( file.isEndWith(*iter,false) )
		{
			return true;
		}
	}
	return false;
}


bool VDirectory::isExcludeFile( const Data& file )
{
	for ( std::vector<Data>::iterator iter=_vecExcludeFile.begin(); iter!=_vecExcludeFile.end(); iter++ )
	{
		if ( file.isEndWith(*iter,false) )
			return true;
	}
	return false;
}

bool VDirectory::isExcludeDir( const Data& dir )
{
	for ( std::vector<Data>::iterator iter=_vecExcludeDir.begin(); iter!=_vecExcludeDir.end(); iter++ )
	{
		if ( dir.isEndWith(*iter,false) )
			return true;
	}
	return false;
}
