
#include "basecode/BaseLog.h"
#include "basecode/VerControl.h"
#include "basecode/directory.h"
#include "basecode/TimeVal.h"
#include "basecode/basefunc.h"
#include "basecode/VFile.h"
#include "basecode/VHttp.h"
#include "basecode/rapidxml.hpp"
#include "basecode/rapidxml_utils.hpp"
#include "basecode/ZipFile.h"


VerControl::VerControl(void)
{
}


VerControl::~VerControl(void)
{
}

std::map<uint32,uint8> g_mapFileHash;
bool VerControl::EnumFile( const Data& filename, const Data& filepath, const Data& relaDir )
{
#ifdef WINDOWS
	if ( filename==String(VER_FILE).c_str() || filename==String(CONFIG_FILE).c_str() )
		return true;
	Data dtLower = filename;
	dtLower.ToLower();
	if ( dtLower!=filename && filename!=VER_FILE && filename!=VER_FILE_INFO )
	{
		VFile::rename( filepath, filename, dtLower );
	}

	Data url = relaDir + filename;
	url.formatPath();
	url.ToLower();
	uint32 hashID = fastHash( url.c_str(), url.length() );
	char *zErrMsg = 0;

	if ( g_mapFileHash.find(hashID) != g_mapFileHash.end() )
	{
		Data dtMsg = Data("文件版本哈希重复:") + url;
		MessageBox( NULL, dtMsg.c_str(), "错误", MB_OK );
		return false;
	}
	g_mapFileHash[hashID] = 0;
	S3_VERFILE_ITEM item;
	item.hashID = hashID;
	item.zip = 0;
	item.fileVer = _ver;
	uint32 len = sizeof(item);
	_file.writeData( &item, sizeof(item) );

	S3_VERFILE_INFO infoItem;
	strcpy( infoItem.name, url.c_str() );
	infoItem.hashID = hashID;
	_infoFile.writeData( &infoItem, sizeof(infoItem) );
#else
    assert(false);
#endif
	return true;

}

bool HashFileFunc( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	VerControl* pClass = (VerControl*)pParam;
	if ( !pClass->EnumFile(filename,filepath,relaDir) )
		return false;
	return true;
}

bool HashFolderFunc( const Data& folderPath, const Data& folder, void* pParam )
{
	Data dtLow = folder;
	dtLow.lowerCase();
	if ( dtLow != folder )
	{
		return VFile::rename( folderPath, folder, dtLow );
		/*
		SHFILEOPSTRUCT fileOp = {0};
		char fromDir[255] = {0,};
		char toDir[255] = {0,};
		Data dtFromPath = folderPath + folder;
		Data dtToPath = folderPath + dtLow;
		strcpy( fromDir, dtFromPath.c_str() );
		strcpy( toDir, dtToPath.c_str() );
		fileOp.pFrom = fromDir;
		fileOp.pTo = toDir;
		fileOp.fFlags = FOF_NOCONFIRMATION;
		fileOp.wFunc = FO_RENAME;
		return (SHFileOperation(&fileOp)==0);
		*/
	}
	return true;
}

bool VerControl::hashFolder( const Data& folder, uint32 ver )
{
	_ver = ver;
	Data strFolder = folder;
	strFolder.makePath();
	VDirectory::createDir( strFolder+"_config/" );
	
	Data strFileName = strFolder + "_config/" + VER_FILE;
	Data strInfoFileName = strFolder + "_config/" + VER_FILE_INFO;
	
	if ( !_file.openFile(strFileName.c_str(),VFile::READWRITE) )
		return false;

	if ( !_infoFile.openFile(strInfoFileName.c_str(),VFile::READWRITE) )
		return false;

	VDirectory dir;
	if ( !dir.EnumAllFunc(strFolder,"",HashFileFunc,HashFolderFunc,this) )
		return false;
	_file.closeFile();
	_infoFile.closeFile();
	return true;
}



bool VerControl::packFile( const Data& filename, const Data& filepath, const Data& relaDir )
{
	VFile file;	
	if ( !file.openFile(filepath+filename,VFile::READ) )
	{
		LogOut( RES, LOG_ERR, "open pack file %s err", filepath.c_str() );
		return false;
	}
	Data tarFolder = _tarFolder + relaDir;
	if ( !VDirectory::isDirectoryExist(tarFolder) )
	{
		if ( !VDirectory::createDir(tarFolder) )
		{
			LogOut( RES, LOG_ERR, "create dir err %s", tarFolder.c_str() );
			return false;
		}
	}

	VFile tarFile;
	Data tarFilePath = tarFolder+filename;
	if ( !tarFile.openFile(tarFilePath,VFile::READWRITE) )
	{
		LogOut( RES, LOG_ERR, "create tar file err %s", tarFilePath.c_str() );
		return false;
	}
	
	if ( !ZipFile::zipFile(file,tarFile) )
	{
		tarFile.closeFile();
		VFile::deleteFile( tarFilePath );
		return false;
	}
	return true;
}


bool PackFileFunc( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	VerControl* pClass = (VerControl*)pParam;
	if ( !pClass->packFile(filename,filepath,relaDir) )
		return false;
	return true;
}

bool VerControl::packFolder( const Data& srcFolder, const Data& tarFolder )
{
	_tarFolder = tarFolder;
	_tarFolder.makePath();
	Data strFolder = srcFolder;
	strFolder.makePath();
	VDirectory dir;
	if ( !dir.EnumAllFunc(strFolder,"",PackFileFunc,NULL,this) )
		return false;
	return true;
}


bool MergFileFunc( const Data& filename, const Data& filepath, const Data& relaDir, void* pParam )
{
	if ( filename.isEqualNoCase(VER_FILE) && relaDir=="_config\\" )
		return true;
	VerControl* pClass = (VerControl*)pParam;
	if ( !pClass->mergFile(filename,filepath,relaDir) )
		return false;
	return true;
}

bool VerControl::mergFile( const Data& filename, const Data& filepath, const Data& relaDir )
{
	Data fileDir = relaDir + filename;
	fileDir.replace( "\\", "/" );
	fileDir.ToLower();
	uint32 hashID = fastHash( fileDir.c_str(), fileDir.length() );

	if ( VFile::isFileExist((_mergTarDir+fileDir).c_str()) )
	{
		std::map<uint32,uint16>::iterator iter;
		if ( (iter=_mapFileHash.find(hashID)) == _mapFileHash.end() )
		{
			String errMsg = String::ToString( "file %s hash not found", fileDir.c_str() );
#ifdef WINDOWS
			MessageBox( NULL, errMsg.c_str(), "错误", MB_OK );
#endif
            assert(false);
			return false;
		}
		else
		{
			iter->second++;
		}
	}
	else
	{
		if ( _mapFileHash.find(hashID) != _mapFileHash.end() )	
		{
			String errMsg = String::ToString( "file %s hash has exist", fileDir.c_str() );
#ifdef WINDOWS
			MessageBox( NULL, errMsg.c_str(), "错误", MB_OK );
#endif
            assert(false);
			return false;
		}
		else
		{
			_mapFileHash[hashID] = 1;
			S3_VERFILE_INFO infoItem;
			memset( &infoItem, 0, sizeof(infoItem) );
			strcpy( infoItem.name, fileDir.c_str() );
			infoItem.hashID = hashID;
			if ( !_infoFile.writeData(&infoItem,sizeof(S3_VERFILE_INFO)) )
			{
#ifdef WINDOWS
				MessageBox( NULL, "写入版本信息文件错误", "错误", MB_OK );
#endif
				return false;
			}
		}
	}
	return true;
}


bool VerControl::mergFolder( const Data& srcDir, const Data& tarDir )
{
	_mergPackDir = srcDir;
	_mergTarDir = tarDir;
	_mergPackDir.makePath();
	_mergTarDir.makePath();
	if ( !VFile::isFileExist(_mergTarDir+"_config/"+VER_FILE_INFO) )
	{
#ifdef WINDOWS
		MessageBox( NULL, "目标版本信息文件不存在", "错误", MB_OK );
#endif
		return false;
	}
	if ( !VFile::copyFile(_mergTarDir+"_config/"+VER_FILE_INFO,_mergPackDir+"_config/"+VER_FILE_INFO) )
	{
#ifdef WINDOWS
		MessageBox( NULL, "版本信息文件复制错误", "错误", MB_OK );
#endif
		return false;
	}
	if ( !_infoFile.openFile(_mergPackDir+"_config/"+VER_FILE_INFO,VFile::APPEND) )
	{
#ifdef WINDOWS
		MessageBox( NULL, "版本信息文件打开错误", "错误", MB_OK );
#endif
		return false;
	}
	if ( !parseVerFile(_mergTarDir+"_config/"+VER_FILE,_mapFileHash) )
		return false;
	VDirectory dir;
	if ( !dir.EnumAllFunc(_mergPackDir,"",MergFileFunc,NULL,this) )
		return false;
	//_file.closeFile();
	VFile verFile;
	VDirectory::createDir( _mergPackDir+"_config/" );
	if ( !verFile.openFile(_mergPackDir+"_config/"+VER_FILE,VFile::READWRITE) )
		return false;
	for ( std::map<uint32,uint16>::iterator iter=_mapFileHash.begin(); iter!=_mapFileHash.end(); iter++ )
	{
		S3_VERFILE_ITEM item;
		item.hashID = iter->first;
		item.fileVer = iter->second;
		item.zip = true;
		if ( !verFile.writeData(&item,sizeof(item)) )
			return false;
	}
	_infoFile.closeFile();
	return true;
}


bool VerControl::parseVerFile( const Data& name, std::map<uint32,uint16>& mapVer )
{
	mapVer.clear();
	VFile verFile;
	if ( !verFile.openFile(name,VFile::READ) )
	{
		LogOut( RES, LOG_ERR, "parse ver file %s err", name.c_str() );
		return false;
	}
	S3_VERFILE_ITEM item;
	while ( !verFile.isEof() )
	{
		memset( &item, 0, sizeof(item) );
		if ( verFile.read((uint8*)&item,sizeof(item)) == sizeof(item) )
		{
			mapVer[item.hashID] = item.fileVer;
		}
	}
	VFile::resetAccessTime( name );
	return true;
}


void VerControl::update( const Data& server )
{
	VHttp http;
	ref_ptr<MemStream> memStream = http.getHttpStream( server, "config.xml" );
	rapidxml::xml_document<> doc;
	doc.parse<0>( (char*)memStream->getPoint());
	rapidxml::xml_node<>* xmlRoot = doc.first_node();
	if ( xmlRoot == NULL )
		return;
	if ( !xmlRoot->compare("s3config") )
		return;
	rapidxml::xml_node<>* subNode = xmlRoot->first_node();
	if ( String(subNode->getName()).equal("filever",String::NoCase) )
	{
		rapidxml::xml_node<>* verNode = subNode->first_node();
		if ( !String(verNode->getName()).equal("version",String::NoCase) )
			return;
		std::string strID;
		if ( !verNode->findAttribute("id",strID) )
			return;
		//int32 ver = String(strID.c_str()).toInt32();		
	}
}
