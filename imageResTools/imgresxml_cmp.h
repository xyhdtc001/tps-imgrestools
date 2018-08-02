#pragma once
#ifndef _IMGRESXML_CMP_H
#define  _IMGRESXML_CMP_H

#include "CXmlOprate.h"
#include <map>

//IMGRES 配置比较. 数据信息.

class CImgresXmlCmp
	:public CXmlOprate
{
	friend CImgresXmlCmp;
public:
	CImgresXmlCmp();
	~CImgresXmlCmp();

	virtual bool InitXmlFile(string strFile);

	//比较2份XML 文件，比较差异.
	int cmpImgresXmlFile(const char* szXmlExPath,bool bImgSetCmp=true);



	const std::map<string, _stImgSetInfo>& getMapInfo() { return m_mapImgInfo; };


private:
	std::map<string, _stImgSetInfo> m_mapImgInfo;

	std::map<string, _stImgSetInfo> diffMapAdd; // 增加的..
	std::map<string, _stImgSetInfo> diffMapDee; //减少的..
};






#endif