#pragma once
#ifndef _IMG_TOOLS_COMMON_H
#define _IMG_TOOLS_COMMON_H


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

	/*
	 *	��ʼ����,Ŀ¼-->��Ӧ��imgset name .xml����.
	 */
	void init_map_dirtoname();

	/*
	 *	��ʼ����runpath��imset��Ŀ¼��.
	 */



	/*
	 *	��ȡres��Ŀ¼��set����.
	 */
	string getSetName(string strDirName);
private:

	std::map<Data, _stImgSetInfo> m_MapDirToName;

};


#endif