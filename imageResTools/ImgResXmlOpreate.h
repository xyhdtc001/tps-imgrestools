#pragma once
#include "imgresxml_cmp.h"
/*
 *	imgxml 处理.
 */

class CImgResXmlOpreate
	:public CImgresXmlCmp
{
public:
	CImgResXmlOpreate();
	~CImgResXmlOpreate();


	//合并tp打包出来的XML
	int joinXmlFile(const char* szXmlExPath,OUT std::map<string, _stImgSetInfo>* pSetInfo=NULL);


	/*
	 *	设置runpath 目录
	 */
	void set_work_dir(const char* szClientPath);

	/*
	 *	设置上次xml
	 */
	void set_lastxml_ptr(CImgResXmlOpreate * lpXmlLast) { m_pLastXml = lpXmlLast; };

	/*
	 *	设置图片的输出目录
	 */
	void set_tp_out_dir(const char * szOutDir);

	/*
	 *	保存文件。重载.
	 */
	virtual bool SaveFile(string strFilePath);

protected:
	/*
	 *	处理3宫图片.
	 */
	int  handle_3g_res(const char* szSetName, _stImgInfo& szImgName, IMGSETMAP& imgMap);

private:
	Data m_dClientDir;

	CImgResXmlOpreate* m_pLastXml;

	Data m_strOutDir;
};

