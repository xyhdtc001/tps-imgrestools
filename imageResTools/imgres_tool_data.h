#pragma once
#ifndef _IMGRES_TOOL_DATA_H
#define _IMGRES_TOOL_DATA_H

#include <map>

// 单图片信息
struct  _stImgInfo
{
	_stImgInfo()
	{
		strName = "";
		nWidth = 0;
		nHeigth = 0;
		bFind = false;
		memset(szMD5,0,65);
	}
	std::string strName;
	int nWidth;
	int nHeigth;
	bool bFind;
	char szMD5[65];
};

typedef std::map<string, _stImgInfo> IMGSETMAP;


// 图片 set 信息.
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
		bFind = false;
	}
	std::string strName;
	std::string strFilePath;
	int nWidth;
	int nHeigth;
	bool bAutoScal;
	bool bOwner;
	IMGSETMAP imgMap;
	bool bFind;
};





#endif