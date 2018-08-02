#pragma once
#ifndef _IMG_TOOLS_COMMON_H
#define _IMG_TOOLS_COMMON_H


/*
 *	单例，非线程安全.
 */

class CImgToolComm
{
private:
	CImgToolComm();
	~CImgToolComm();


	static CImgToolComm * m_signleInstance;

public:

	static CImgToolComm* GetSignleInstance();

	/*
	 *	初始化表,目录-->对应的imgset name .xml配置.
	 */
	void init_map_dirtoname();

	/*
	 *	初始化，runpath下imset的目录名.
	 */



	/*
	 *	获取res的目录的set名字.
	 */
	string getSetName(string strDirName);
private:

	std::map<Data, _stImgSetInfo> m_MapDirToName;

};


#endif