#pragma once
#ifndef _IMG_TOOLS_COMMON_H
#define _IMG_TOOLS_COMMON_H

#include <set>
/*
 *	���������̰߳�ȫ.
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
	 *	���ù���Ŀ¼
	 */
	void setWorkDir(LPCSTR szWorkDir);

	/*
	 *	��ʼ����,Ŀ¼-->��Ӧ��imgset name .xml����.
	 */
	void init_map_dirtoname();


	/*
	 *	3����ͼƬ��Դ��.
	 */
	void init_map_3gres();


	/*
	 *	��ʼ����runpath��imset��Ŀ¼��.
	 */



	/*
	 *	��ȡres��Ŀ¼��set����.
	 */
	string getSetName(string strDirName);


	/*
	 *	�Ƿ���3����ͼƬ.
	 */
	bool is_3g_res(string strSetname,string strImgName);
private:

	std::map<Data, _stImgSetInfo> m_MapDirToName;
	std::set<Data> m_Map3gRes;
	Data m_workDir;
};


#endif