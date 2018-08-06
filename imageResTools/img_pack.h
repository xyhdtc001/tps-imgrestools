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
	 *	������Ŀ¼.
	 */
	void setMainDir(LPCSTR szPackSourcePath);

	/*
	 *	���ú���汾Ŀ¼. 
	 */
	void setLanResDir(LPCSTR szPackSourcePath);

	/*
	 *	����Texure-pack exe ·��.
	 */
	void setTexturepackExePath(const char* szExePath);


	/*
	 *	�������Ŀ¼.
	 */
	void setOutPutDir(const char* szOutPutDir);

	/*
	 *	����XML ָ��
	 */
	void setXmlPtr(CImgresXmlCmp *lpXmlCur, CImgresXmlCmp* lpXmlLast);


/************************************************************************/
/*  ҵ��ӿ�                                                            */
/************************************************************************/

	/*
	 *	��ʼ���.
	 */
	bool startPack(bool bChildDir = true);


	/*
	 *	��ȡ����ɹ���Ŀ¼.
	 */
	const std::vector<Data>& getPackDirVec() { return m_curVecOutPut; };

protected:

	/*
	 *	����Ŀ¼����.
	 */
	bool PackSignDir(const Data& folderPath);

	/*
	 *	��Ŀ¼���.ʹ�� teurepack
	 */
	DWORD imp_pack(LPCSTR szPackSourcePath);

	/*
	 *	��ȡ����Ŀ¼dir num
	 */
	int get_work_dir_num() {
		return m_nWorkDirNum;
	};

	/*
	 *	����ļ���.
	 */
	int img_pack_split(Data dToSplit, Data& vecNewEx);


	/*
	 *	����ͼƬ
	 */
	int img_pack_calc(Data dAbsDir,std::vector<Data> vecFiles,OUT std::vector<Data>& vecResFiles);

	/*
	 *	�Ա�2��ͼ��ͼƬ.B�б�A�������ͼƬ.
	 */
	 int img_cmp_set_add(const _stImgSetInfo &imgSetA, _stImgSetInfo& imgSetB,OUT std::vector<Data>& vecRes);
	 int  img_cmp_set_add(const _stImgSetInfo &imgSetA, std::vector<Data>& imgFileB,OUT std::vector<Data>& vecRes);

private:
	Data m_strTexturePackExePath; //texture pack ·��.

	Data m_strMainResDir; //��Ҫ����Ŀ¼.

	Data m_strLanResDir;//��ԴĿ¼.

	Data m_strOutPutDir; //���Ŀ¼.

	bool m_bChildDir;  //��Ŀ¼��� 

	bool m_bLanResPack;//������Դ���?

	int m_nDeep; //��Ŀ¼���.

	int m_nWorkDirNum; //Ŀ¼���.


	bool m_bSyncLanRes;// �Ƿ�ͬ������汾.

	struct _st_imgResInfo
	{
		std::string strDir;//���Ŀ¼.
		DWORD dwLastChangeTime; //�ϴ��޸�ʱ��.
		
	};

	CImgresXmlCmp *m_pXmlCur; //��ǰXML
	CImgresXmlCmp *m_pXmlLast;//�ϴ�XML
	std::vector<Data> m_curVecOutPut;
};


#endif