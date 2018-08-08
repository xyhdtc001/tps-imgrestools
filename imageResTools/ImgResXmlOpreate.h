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
	int joinXmlFile(const char* szXmlExPath);


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

protected:
	/*
	 *	����3��ͼƬ.
	 */
	int  handle_3g_res(const char* szSetName, _stImgInfo& szImgName, IMGSETMAP& imgMap);

private:
	Data m_dClientDir;

	CImgResXmlOpreate* m_pLastXml;

	Data m_strOutDir;
};

