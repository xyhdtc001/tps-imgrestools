#pragma once
#include "imgresxml_cmp.h"
/*
 *	imgxml ����.
 */

class CImgResXmlOpreate
	:public CImgresXmlCmp
{
public:
	CImgResXmlOpreate();
	~CImgResXmlOpreate();


	//�ϲ�tp���������XML
	int joinXmlFile(const char* szXmlExPath,OUT std::map<string, _stImgSetInfo>* pSetInfo=NULL);


	/*
	 *	����runpath Ŀ¼
	 */
	void set_work_dir(const char* szClientPath);

	/*
	 *	�����ϴ�xml
	 */
	void set_lastxml_ptr(CImgResXmlOpreate * lpXmlLast) { m_pLastXml = lpXmlLast; };

	/*
	 *	����ͼƬ�����Ŀ¼
	 */
	void set_tp_out_dir(const char * szOutDir);

	/*
	 *	�����ļ�������.
	 */
	virtual bool SaveFile(string strFilePath);

	/*
	 *	����xml��3gλ����Ϣ���ظ�������Ϣ.
	 */
	void update_img_info();


	/*
	 *	��ȡ���е�3g��Ϣ.
	 */
	bool get_3g_info(string strName, _stImgInfo& stInfo,string strSet);
protected:
	/*
	 *	����3��ͼƬ.
	 */
	int  handle_3g_res(const char* szSetName, _stImgInfo& szImgName, IMGSETMAP& imgMap);

private:
	Data m_dClientDir;

	CImgResXmlOpreate* m_pLastXml;

	Data m_strOutDir;


	std::map<string, std::vector<_stImgInfo>> m_map3GInfo;
	std::map<string, std::vector<_stImgInfo>> m_mapAllImgageInfo;
};

