#pragma once
#ifndef _SYNC_HANDLE_H
#define _SYNC_HANDLE_H
#include "basecode/StringData.h"

#include <map>
#include <set>


typedef std::set<string> SETSTRING;
struct _sysFileInfo
{
	SETSTRING setPaths;//��������!
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

	//������Ŀ¼.
	void setWorkDir(const char * strWorkDir);

	//ͬ������.
	bool sysDir(std::string strStandDir, std::string strToChangeDir);

	//��ʼ������.
	void initAppData();


	//
	bool isHandleExt(const char * szExt);

	//����ļ���.(1���ļ��в��2���ļ���.)
	static int split_dir(std::vector<Data> strFile,Data dDir, Data &dDirNewExport);

private:

	std::string m_strWorkDir;
	SETSTRING m_strExtFileName;
};


#endif