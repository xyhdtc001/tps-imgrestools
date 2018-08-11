#pragma once
#ifndef _IMGRES_TOOL_DATA_H
#define _IMGRES_TOOL_DATA_H

#include <map>


//Ŀ¼��Ϣ.
#define		DIR_IMGRESXML_STR			"data/ui/resourcexml/"
#define		FILE_IMGRESXML_STR			"data/ui/resourcexml/imagesetresource.xml"
#define     FILE_LASTIMGRES_STR         "last_imgresset.xml"

// ��ͼƬ��Ϣ
struct  _stImgInfo
{
	_stImgInfo()
	{
		strName = "";
		nWidth = 0;
		nHeigth = 0;
		nPosX = -1;
		nPosY = -1;
		memset(szMD5,0,65);
		strSetName = "";
	}
	std::string strSetName;
	std::string strName;
	int nWidth;
	int nHeigth;
	char szMD5[65];
	int nPosX;
	int nPosY;
};

typedef std::map<string, _stImgInfo> IMGSETMAP;


// ͼƬ set ��Ϣ.
struct  _stImgSetInfo
{
	_stImgSetInfo()
	{
		strName = "";
		strFilePath = "";
		nWidth = 0;
		nHeigth = 0;
		bAutoScal = false;
		bOwner = true;
		imgMap.clear();
	}
	std::string strName;
	std::string strFilePath;
	int nWidth;
	int nHeigth;
	bool bAutoScal;
	bool bOwner;
	IMGSETMAP imgMap;
};



#endif