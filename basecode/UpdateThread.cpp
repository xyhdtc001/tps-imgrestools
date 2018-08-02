#include "basecode/rapidxml.hpp"
#include "basecode/rapidxml_iterators.hpp"
#include "basecode/rapidxml_utils.hpp"
#include "basecode/rapidxml_print.hpp"

#include "basecode/directory.h"
#include "basecode/baselog.h"
#include "basecode/VFile.h"
#include "basecode/VerControl.h"
#include "basecode/BaseLog.h"
#include "basecode/ZipFile.h"
#include "basecode/UpdateThread.h"
#include "basecode/VNameMutex.h"
#include "basecode/ClientCryptor.h"

const int s3_update_file_identity = 0x0915;
const int local_file_buffer_size = 100000;


UpdateThread::UpdateThread() : _encClient(0x20, 0xFD, 0x07, 0x1F, 0x7A, 0xCF, 0xE5, 0x3F)
{
	_bShowPic = true;
	_configFile = CONFIG_FILE;
	_bUpdateAll = false;
	_downloadHttp.setNotifyCallback( this );
	_downloadHttp.enableSpeedCheck();
}

UpdateThread::~UpdateThread(void)
{
}

void
UpdateThread::updateAll()
{
	_bUpdateAll = true;
}

#if defined WINDOWS
void UpdateThread::setWnd( HWND hWnd )
{
	_hWnd = hWnd;
}
#else
#if defined METRO

#else
#endif
#endif


void
UpdateThread::thread()
{
	LogDebug( RES, "Start work thread" );
	
	VDirectory::createDir( _localDir );
	Data localConfigFile = _localDir + CONFIG_FILE;
	localConfigFile.formatPath();

	VNameMutex mutex;
	mutex.create( localConfigFile.c_str() );
	VNameMutexLockHelper helper( &mutex );

	ref_ptr<MemStream> configMemStream = _http.getHttpStream( _serverIp, _serverFolder+_configFile, -1, 999 );	
	if ( configMemStream == NULL )
	{
		LogOut( RES, LOG_ERR, "Get config file err" );
		raiseError( "获取版本错误，请检查网络重试" );
		return;
	}

	if ( _bShowPic )
	{
		Data loadPicName = _localDir + LOADING_PIC;
		int32 len = -1;
		if ( VFile::isFileExist(loadPicName) )
			len = VFile::getFileLen( loadPicName );
		
		ref_ptr<MemStream> loadPicMemStream = _http.getHttpStream( _serverIp, _serverFolder+LOADING_PIC, -1, 999 );
		if ( loadPicMemStream != NULL )
		{
			//重新下载图片
			VFile loadPicFile;
			if ( loadPicFile.openFile(loadPicName, VFile::READWRITE) )
			{
				if ( loadPicFile.writeData(loadPicMemStream->getPoint(),loadPicMemStream->getStreamSize()) )
				{
					loadPicFile.closeFile();
#ifdef WINDOWS
#ifndef METRO
					::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_SHOW_LOADING_PIC, 0 );
#endif
#endif
				}
			}
		}
		else if ( len > 0 )
		{
#ifdef WINDOWS
#ifndef METRO
			::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_SHOW_LOADING_PIC, 0 );
#endif
#endif
		}
	}
	
	
	/*
	Data waitPicName = _localDir + WAITING_PIC;
	len = -1;
	if ( VFile::isFileExist(waitPicName) )
		len = VFile::getFileLen( waitPicName );
	
	ref_ptr<MemStream> waitPicMemStream = http.getHttpFileStream( _serverIp, _serverFolder+"_config/"+WAITING_PIC );
	if ( waitPicMemStream != NULL )
	{
		VFile waitPicFile;
		if ( waitPicFile.openFile(waitPicName, VFile::READWRITE) )
		{
			if ( waitPicFile.writeData(waitPicMemStream->getPoint(),waitPicMemStream->getStreamSize()) )
			{
				waitPicFile.closeFile();
				#ifdef WINDOWS
				::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_SHOW_WAITING_PIC, 0 );
				#endif
			}
		}
	}
	else if ( len > 0 )
	{
		#ifdef WINDOWS
		::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_SHOW_WAITING_PIC, 0 );
		#endif
	}
	*/

	LogDebug( RES, "Get config file ok" );
	ref_ptr<MemStream> memStreamBak = configMemStream->clone();
	rapidxml::xml_document<> doc;
	doc.parse<0>( (char*)configMemStream->getPoint() );
	parseConfig( doc, _remoteVersion, false );
	if ( !_bUpdateAll )
	{
		VFile localFile;
		if ( !localFile.openFile(localConfigFile,VFile::READ) )
		{
			LogDebug( RES, "havn't local config file" );
		}
		else
		{
			LogDebug( RES, "Open local config file ok: %s", localConfigFile.c_str() );
			S3_SAFE_FILE_HDR hdr;
			if ( localFile.read((uint8*)&hdr,sizeof(hdr)) == sizeof(hdr) )
			{
				if ( hdr.identity==s3_update_file_identity && hdr.status==FILE_OK )
				{
					char buff[local_file_buffer_size] = {0,};
					if ( localFile.read((uint8*)buff,hdr.fileNum) == hdr.fileNum )
					{
						_encClient.Decrypt( (uint8*)buff, hdr.fileNum );
						doc.parse<0>( buff );
						parseConfig( doc, _localVersion, true );
					}
				}
			}
			localFile.closeFile();
		}
	}
	
	std::vector<FileItem> vecUpdateItem;
	for ( std::vector<FileItem>::iterator iter=_remoteVersion.vecFile.begin(); iter!=_remoteVersion.vecFile.end(); iter++ )
	{
		FileItem* pItem = NULL;
		for ( std::vector<FileItem>::iterator it=_localVersion.vecFile.begin(); it!=_localVersion.vecFile.end(); it++ )
		{
			if ( iter->name == it->name )
			{
				Data dtFile;
				if ( iter->path.isEmpty() )
				{
					dtFile = _localDir + iter->file.getFileName();
				}
				else
				{
					dtFile = _localDir + iter->path;
					dtFile.makePath();
					dtFile += iter->file.getFileName();
				}
				if ( VFile::isFileExist(dtFile) )
				{
					pItem = &(*it);
				}
				break;
			}
		}
		if ( pItem == NULL )
		{
			vecUpdateItem.push_back( *iter );
		}
		else
		{
			if ( iter->ver > pItem->ver )
			{
				LogDebug( RES, "remote %s version %d update local version %d", iter->name.c_str(), iter->ver, pItem->ver );
				vecUpdateItem.push_back( *iter );
			}
		}
	}

	uint32 index = 0;
	for ( std::vector<FileItem>::iterator iter=vecUpdateItem.begin(); iter!=vecUpdateItem.end(); iter++ )
	{
		++index;

#ifdef WINDOWS
		uint32 lParam = (uint32)( (vecUpdateItem.size()<<16)+index );
#ifndef METRO
		PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_NEW_FILE, lParam );	
#endif
#endif

		if ( !downloadFileItem(&(*iter)) )
		{
			LogOut( MAIN, LOG_ERR, "install file item %s err", iter->name.c_str() );
			raiseError( "下载版本错误" );
			return;
		}
	}

	if ( vecUpdateItem.size() > 0 )
	{
		::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_UPDATE, 0 );
		VFile::deleteFile( localConfigFile );
	}

	//VFile::deleteFile( localConfigFile );
	for ( std::vector<FileItem>::iterator iter=vecUpdateItem.begin(); iter!=vecUpdateItem.end(); iter++ )
	{
		if ( installFileItem(&(*iter)) )
		{
			updateClientItem( &(*iter) );
		}
		else
		{
			LogOut( MAIN, LOG_ERR, "unzip file item %s err", iter->name.c_str() );
			raiseError( "解压出错" );
			return;
		}
	}

	if ( vecUpdateItem.size() > 0 )
	{
		LogDebug( RES, "update local config file: %s", localConfigFile.c_str() );
		rapidxml::xml_document<> doc;
		rapidxml::xml_node<>* rootNode = doc.allocate_node( rapidxml::node_element, "s3ver" );
		doc.append_node( rootNode );
		rapidxml::xml_node<>* progNode = doc.allocate_node( rapidxml::node_element, "programe" );
		rootNode->append_node( progNode );
		for ( std::vector<FileItem>::iterator iter=_localVersion.vecFile.begin(); iter!=_localVersion.vecFile.end(); iter++ )
		{
			LogDebug( RES, "write file config: %s", iter->file.c_str() );
			rapidxml::xml_node<>* fileNode = doc.allocate_node( rapidxml::node_element, "file" );
			fileNode->append_attribute( doc.allocate_attribute("ver",strdup(Data::ToString(iter->ver).c_str())) );
			fileNode->append_attribute( doc.allocate_attribute("name",strdup(iter->name.c_str())) );
			fileNode->append_attribute( doc.allocate_attribute("file",strdup(iter->file.c_str())) );
			if ( !iter->path.isEmpty() )
				fileNode->append_attribute( doc.allocate_attribute("path",strdup(iter->path.c_str())) );
			progNode->append_node( fileNode );
		}
		char outBuff[local_file_buffer_size] = {0,};
		char* pEnd = print( outBuff, doc );
		//uint32 fileLen = (uint32)(pEnd-outBuff);
		size_t fileLen = pEnd-outBuff; 
		_encClient.Encrypt( (uint8*)outBuff, (size_t&)fileLen );

		S3_SAFE_FILE_HDR hdr;
		memset( &hdr, 0, sizeof(hdr) );
		hdr.identity = s3_update_file_identity;
		VFile file;
		if ( !file.openFile(localConfigFile,VFile::READWRITE) )
		{
			raiseError( "更新版本文件错误" );
			return;
		}
		hdr.fileNum = (uint32)fileLen;
		hdr.status = FILE_OPERATING;
		if ( !file.writeData(&hdr,sizeof(hdr)) )
		{
			raiseError( "版本文件写入错误" );
			return;
		}
		if ( !file.writeData(outBuff,(uint32)fileLen) )
		{
			raiseError( "版本文件写入错误" );
			return;
		}
		hdr.status = FILE_OK;
		file.seekTo( 0, SEEK_SET );
		if ( file.write((uint8*)&hdr,sizeof(S3_SAFE_FILE_HDR)) != sizeof(S3_SAFE_FILE_HDR) )
		{
			raiseError( "版本文件写入错误" );
			return;
		}
	}
	else
	{
		LogDebug( RES, "Not update item" );
	}
    
	//更新或下载完成
#ifdef WINDOWS
#ifndef METRO
	::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_OK, 0 );
#endif
#endif
}


void UpdateThread::updateClientItem( FileItem* pItem )
{
	bool bFind = false;
	for ( std::vector<FileItem>::iterator iter=_localVersion.vecFile.begin(); iter!=_localVersion.vecFile.end(); iter++ )
	{
		if ( iter->name == pItem->name )
		{
			*iter = *pItem;
			bFind = true;
			break;
		}
	}
	if ( !bFind )
	{
		_localVersion.vecFile.push_back( *pItem );
	}
}


void UpdateThread::parseConfig( rapidxml::xml_document<>& doc, VersionItem& item, bool bLocal )
{
	rapidxml::xml_node<>* xmlRoot = doc.first_node();
	if ( xmlRoot == NULL )
		return;
	if ( !xmlRoot->compare("s3ver") )
		return;
	rapidxml::xml_node<>* subNode = xmlRoot->first_node();
	String subNodeName = subNode->getName();
	if ( subNodeName.equal("programe",String::NoCase) )
	{
		for ( rapidxml::xml_node<>* fileNode = subNode->first_node();fileNode;fileNode=fileNode->next_sibling() )
		{
			FileItem fileItem;
			if ( String(fileNode->getName()).equal("file",String::NoCase) )
			{
				std::string strAttr;
				String strDest,strFile,strDestPath;
				if ( !fileNode->findAttribute("name",strAttr) )
					continue;
				fileItem.name = strAttr.c_str();

				if ( !fileNode->findAttribute("ver",strAttr) )
					continue;
				int32 ver = String(strAttr.c_str()).toInt32();
				fileItem.ver = ver;

				if ( !fileNode->findAttribute("file",strAttr) )
					continue;
				fileItem.file = strAttr.c_str();
				//fileItem.file.lowerCase();

				if ( fileNode->findAttribute("init",strAttr) )
					fileItem.bInit = true;

				if ( fileNode->findAttribute("path",strAttr) )
				{
					fileItem.path = strAttr.c_str();
				//	fileItem.path.lowerCase();
				}


				if ( bLocal )
					LogDebug( RES, "Add local item %s ver %d", fileItem.name.c_str(), fileItem.ver );
				else
					LogDebug( RES, "Add remote item %s ver %d", fileItem.name.c_str(), fileItem.ver );
				item.vecFile.push_back( fileItem );
			}
		}
	}
}


bool UpdateThread::downloadFileItem( FileItem* pItem )
{
	LogDebug( RES, "download item %s", pItem->name.c_str() );
	ref_ptr<MemStream> memStream = _downloadHttp.getHttpStream( _serverIp, _serverFolder+pItem->file, -1, 999 );
	if ( memStream == NULL )
	{
		LogDebug( RES, "download item %s error", pItem->name.c_str() );
		return false;
	}
	pItem->memStream = memStream;
	return true;
}


typedef bool (*InitFunc)(void);
//typedef HRESULT (*RegisterFunc)(void);

bool UpdateThread::installFileItem( FileItem* pItem )
{
	if ( pItem->memStream == NULL )
	{
		raiseError( "数据为空" );
		return false;
	}
	
	Data destFile;
	if ( pItem->path.isEmpty() )
	{
		destFile = _localDir + pItem->file.getFileName();
	}
	else
	{
		destFile = _localDir + pItem->path;
		destFile.makePath();
		VDirectory::createDir( destFile );
		destFile += pItem->file.getFileName();
	}

	ref_ptr<MemStream> tarStream = ZipFile::unZipStream( pItem->memStream );
	if ( tarStream == NULL )
	{
		LogOut( RES, LOG_ERR, "unzip %s err", pItem->name.c_str() );
		raiseError( "解包错误" );
		return false;
	}

	VFile file;
	if ( !file.openFile(destFile,VFile::READWRITE) )
	{
		LogOut( RES, LOG_ERR, "Open file %s err", destFile.c_str() );
		raiseError( "更新错误,请关闭所有浏览器或注销后重试" );
		return false;
	}
	if ( !file.write((uint8*)tarStream->getPoint(),tarStream->getStreamSize())  )
	{
		LogOut( RES, LOG_ERR, "Write file %s err", destFile.c_str() );
		raiseError( "文件写入错误,请关闭所有浏览器或注销后重试" );
		return false;
	}
	file.closeFile();
	LogDebug( RES, "write file ok: %s", destFile.c_str() );

	if ( pItem->bInit )
	{
		#ifdef WINDOWS
		#ifndef METRO
		LogDebug( RES, "init item %s", pItem->name.c_str() );
		HMODULE hLib = LoadLibrary( destFile.c_str() );
		if ( hLib == NULL )
		{
			LogOut( RES, LOG_ERR, "load library %s err", destFile.c_str() );
			raiseError( "加载错误" );
			return false;
		}
		InitFunc func;
		func = (InitFunc)GetProcAddress( hLib, "plugin_init" );
		if ( func == NULL )
		{
			LogOut( RES, LOG_ERR, "get reg func err: %s", destFile.c_str() );
			raiseError( "调用错误1" );
			return false;
		}
		bool bRet = func();
		FreeLibrary( hLib );
		if ( !bRet )
		{
			LogOut( RES, LOG_ERR, "register err: %s", destFile.c_str() );
			raiseError( "调用错误2" );
			return false;
		}
		LogDebug( RES, "调用 %s ok", pItem->name.c_str() );
		#endif
		#else
        assert(false);
		#endif
	}
	return true;
}


void UpdateThread::setDir( const Data& dir )
{
	_localDir = dir;
	_localDir.makePath();
}


void UpdateThread::setServer( const Data& server )
{
	Data serverIp,serverFolder;
	int32 pos = server.find( "/" );
	if ( pos != -1 )
	{
		_serverIp = server.substr( 0, pos );
		_serverFolder = server.substr( pos );
		_serverFolder.makePath();
	}
	else
	{
		_serverIp = server;
		_serverFolder = "/";
	}
}


void UpdateThread::progress( uint32 percent )
{
#ifdef WINDOWS
#ifndef METRO
	::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_PROGRESS, percent );	
#endif
#endif
}


uint32 UpdateThread::getSpeed()
{
	return _downloadHttp.getSpeed();
}


const char* UpdateThread::getErrMsg()
{
	if ( _errMsg.isEmpty() )
		return NULL;
	else
		return _errMsg.c_str();
}


void UpdateThread::setErrMsg( const Data& msg )
{
	_errMsg = msg;
}


void UpdateThread::raiseError( const Data& msg )
{
	_errMsg = msg;
	#ifdef WINDOWS
	#ifndef METRO
	::PostMessage( _hWnd, WM_UPDATE_EVENT, UPDATE_ERR, 0 );
	#endif
#endif
}


void UpdateThread::shutdown()
{
	_http.close();
	_downloadHttp.close();
	ThreadIf::shutdown();
	join();
}

void UpdateThread::setShowPic( bool bShow )
{
	_bShowPic = bShow;
}

void UpdateThread::sethInstance( HINSTANCE inst )
{
	_hInst = inst;
}