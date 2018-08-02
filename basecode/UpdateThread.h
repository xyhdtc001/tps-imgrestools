#pragma once

#include <vector>
#include "basecode/basehdr.h"
#include "basecode/rapidxml.hpp"
#include "basecode/rapidxml_utils.hpp"
#include "basecode/threadif.h"
#include "basecode/stringdata.h"
#include "basecode/memstream.h"
#include "basecode/ref_ptr.h"
#include "basecode/VHttp.h"
#include "basecode/ClientCryptor.h"

#define WM_UPDATE_EVENT WM_USER+1
//#define WM_STOP_WAITING WM_USER+2


//typedef CEncryptClient<0x80, 0x09, 0x15, 0x83, 0x08, 0x26, 0x12, 0x02> ENCRYPTCLIENT;

enum UpdateState
{
	UPDATE_OK = 0,
	UPDATE_ERR,
	UPDATE_NEW_FILE,
	UPDATE_PROGRESS,
	UPDATE_SHOW_LOADING_PIC,
	UPDATE_SHOW_WAITING_PIC,
	UPDATE_UPDATE,
};


struct FileItem
{
	FileItem() : ver(0), bInit(false),bRegister(false)
	{}
	int ver;
	Data name;
	Data file;
	Data path;
	String destDir;
	bool bInit;
	bool bRegister;
	ref_ptr<MemStream> memStream;
};

struct VersionItem
{
	std::vector<FileItem> vecFile;
};


class UpdateThread : public ThreadIf, public IHttpCallback
{

public:
	UpdateThread();
	~UpdateThread(void);
	void setDir( const Data& dir );
	void setServer( const Data& server );

	#if defined WINDOWS
	void setWnd( HWND hWnd );
	#else
	#if defined METRO
	#else
	#endif
	#endif
	void updateAll();
	const char* getErrMsg();
	void setErrMsg( const Data& msg );
	void raiseError( const Data& msg );
	uint32 getSpeed();
	void shutdown();
	void sethInstance( HINSTANCE inst );
	void setShowPic( bool bShow );

protected:
	void thread();
	void parseConfig( rapidxml::xml_document<>& doc, VersionItem& item, bool bLocal );
	bool downloadFileItem( FileItem* pItem );
	bool installFileItem( FileItem* pItem );
	void updateClientItem( FileItem* pItem );
	void progress( uint32 percent );

protected:
	VersionItem _localVersion;
	VersionItem _remoteVersion;
	Data _localDir;
	Data _serverIp,_serverFolder;
	Data _configFile;
	
	bool _bUpdateAll;
	CEncryptClient _encClient;
	Data _errMsg;
	VHttp _downloadHttp;
	VHttp _http;
	HINSTANCE _hInst;
	bool _bShowPic;

	#if defined WINDOWS
	HWND _hWnd;
	#else
	#if defined METRO
	#else
	#endif
	#endif
};
