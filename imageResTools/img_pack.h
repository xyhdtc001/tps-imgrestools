#pragma once
#ifndef _IMG_PACK_H
#define _IMG_PACK_H



class CImgresXmlCmp;
class CImgPack
{
	friend bool enum_dir_callback(const Data& folderPath, const Data& folder, void* pParam);
public:
	CImgPack();
	~CImgPack();

	/*
	 *	设置主目录.
	 */
	void setMainDir(LPCSTR szPackSourcePath);

	/*
	 *	设置海外版本目录. 
	 */
	void setLanResDir(LPCSTR szPackSourcePath);

	/*
	 *	设置Texure-pack exe 路径.
	 */
	void setTexturepackExePath(const char* szExePath);


	/*
	 *	设置输出目录.
	 */
	void setOutPutDir(const char* szOutPutDir);

	/*
	 *	设置XML 指针
	 */
	void setXmlPtr(CImgresXmlCmp *lpXmlCur, CImgresXmlCmp* lpXmlLast);


/************************************************************************/
/*  业务接口                                                            */
/************************************************************************/

	/*
	 *	开始打包.
	 */
	bool startPack(bool bChildDir = true);


	/*
	 *	获取打包成功的目录.
	 */
	const std::vector<Data>& getPackDirVec() { return m_curVecOutPut; };

protected:

	/*
	 *	单个目录处理.
	 */
	bool PackSignDir(const Data& folderPath);

	/*
	 *	单目录打包.使用 teurepack
	 */
	DWORD imp_pack(LPCSTR szPackSourcePath);

	/*
	 *	获取工作目录dir num
	 */
	int get_work_dir_num() {
		return m_nWorkDirNum;
	};

	/*
	 *	拆分文件夹.
	 */
	int img_pack_split(Data dToSplit, Data& vecNewEx);


	/*
	 *	计算图片
	 */
	int img_pack_calc(Data dAbsDir,std::vector<Data> vecFiles,OUT std::vector<Data>& vecResFiles);

	/*
	 *	对比2个图集图片.B中比A多出来的图片.
	 */
	 int img_cmp_set_add(const _stImgSetInfo &imgSetA, _stImgSetInfo& imgSetB,OUT std::vector<Data>& vecRes);
	 int  img_cmp_set_add(const _stImgSetInfo &imgSetA, std::vector<Data>& imgFileB,OUT std::vector<Data>& vecRes);

private:
	Data m_strTexturePackExePath; //texture pack 路径.

	Data m_strMainResDir; //主要工作目录.

	Data m_strLanResDir;//资源目录.

	Data m_strOutPutDir; //输出目录.

	bool m_bChildDir;  //子目录打包 

	bool m_bLanResPack;//海外资源打包?

	int m_nDeep; //子目录深度.

	int m_nWorkDirNum; //目录深度.


	bool m_bSyncLanRes;// 是否同步海外版本.

	struct _st_imgResInfo
	{
		std::string strDir;//相对目录.
		DWORD dwLastChangeTime; //上次修改时间.
		
	};

	CImgresXmlCmp *m_pXmlCur; //当前XML
	CImgresXmlCmp *m_pXmlLast;//上次XML
	std::vector<Data> m_curVecOutPut;
};


#endif