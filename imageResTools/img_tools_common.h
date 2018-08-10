#pragma once
#ifndef _IMG_TOOLS_COMMON_H
#define _IMG_TOOLS_COMMON_H

#include <set>
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

	static void ReleaseInstance();

	/*
	 *	设置工作目录
	 */
	void setWorkDir(LPCSTR szWorkDir);

	/*
	 *	初始化表,目录-->对应的imgset name .xml配置.
	 */
	void init_map_dirtoname();


	/*
	 *	3宫格图片资源表.
	 */
	void init_map_3gres();


	/*
	 *	初始化，runpath下imset的目录名.
	 */



	/*
	 *	获取res的目录的set名字.
	 */
	string getSetName(string strDirName);


	/*
	 *	是否是3宫格图片.
	 */
	bool is_3g_res(string strSetname,string strImgName);
private:

	std::map<Data, _stImgSetInfo> m_MapDirToName;
	std::set<Data> m_Map3gRes;
	Data m_workDir;
};


#endif