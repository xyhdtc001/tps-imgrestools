#pragma once
#ifndef _IMGRESXML_CMP_H
#define  _IMGRESXML_CMP_H

#include "CXmlOprate.h"
#include <map>

//IMGRES ���ñȽ�. ������Ϣ.

class CImgresXmlCmp
	:public CXmlOprate
{
	friend CImgresXmlCmp;
public:
	CImgresXmlCmp();
	~CImgresXmlCmp();

	virtual bool InitXmlFile(string strFile);

	//�Ƚ�2��XML �ļ����Ƚϲ���.
	int cmpImgresXmlFile(const char* szXmlExPath,bool bImgSetCmp=true);



	const std::map<string, _stImgSetInfo>& getMapInfo() { return m_mapImgInfo; };


private:
	std::map<string, _stImgSetInfo> m_mapImgInfo;

	std::map<string, _stImgSetInfo> diffMapAdd; // ���ӵ�..
	std::map<string, _stImgSetInfo> diffMapDee; //���ٵ�..
};






#endif