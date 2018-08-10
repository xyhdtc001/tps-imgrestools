#pragma once
#ifndef _SYNC_HANDLE_H
#define _SYNC_HANDLE_H
#include "basecode/StringData.h"

#include <map>
#include <set>


typedef std::set<string> SETSTRING;
struct _sysFileInfo
{
	SETSTRING setPaths;//重名兼容!
};



class CSyncHandle;
struct _sysFileParam
{
	_sysFileParam()
	{
		dStanDir = "";
		dDesDir = "";
		mapStandFilePathInfo.clear();
		mapTocFilePathInfo.clear();
		nCurScanState = 0;
		ptr_syncHandle = NULL;
	}
	Data dStanDir;
	Data dDesDir;
	std::map<string, _sysFileInfo>mapStandFilePathInfo;
	std::map<string, _sysFileInfo>mapTocFilePathInfo;
	int nCurScanState;
	CSyncHandle * ptr_syncHandle;
};


class CImgResXmlOpreate;
class  CSyncHandle
{
	friend bool scanfiletoxml(const Data &filename, const Data& filepath, const Data& relaDir, void *pParam);
public:
	 CSyncHandle();
	~ CSyncHandle();

	//主工作目录.
	void setWorkDir(const char * strWorkDir);

	//同步操作.
	bool sysDir(std::string strStandDir, std::string strToChangeDir);

	//初始化数据.
	void initAppData();


	//
	bool isHandleExt(const char * szExt);

	//拆分文件夹.(1个文件夹拆成2个文件夹.)
	static int split_dir(std::vector<Data> strFile,Data dDir, Data &dDirNewExport);

private:

	std::string m_strWorkDir;
	SETSTRING m_strExtFileName;
};


#endif