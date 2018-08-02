#include "stdafx.h"
#include "img_pack.h"


#include "basecode/VFile.h"
#include "basecode/directory.h"
#include "ximg_wrap.h"
#include "imgresxml_cmp.h"
#include "img_tools_common.h"
#include "sync_handle.h"
#include "ximg_wrap.h"

CImgPack::CImgPack()
{
	m_strTexturePackExePath = "";
	m_strMainResDir = "";
	m_strLanResDir = "";
	m_strOutPutDir = "";
	m_bChildDir = true;
	m_nDeep = 1;
	m_bLanResPack = false;
	m_nWorkDirNum = 0;
}

CImgPack::~CImgPack()
{
}



void CImgPack::setMainDir(LPCSTR szPackSourcePath)
{
	if (szPackSourcePath)
	{
		m_strMainResDir = szPackSourcePath;
		m_strMainResDir.makePath();
		m_strMainResDir.formatPath();

		VDirectory vd(m_strMainResDir);
		m_nWorkDirNum = vd.getDirNum();
	}
}

void CImgPack::setLanResDir(LPCSTR szPackSourcePath)
{
	if (szPackSourcePath)
	{
		m_strLanResDir = szPackSourcePath;
		m_strLanResDir.makePath();
		m_strLanResDir.formatPath();
	}
}

DWORD CImgPack::imp_pack(LPCSTR szPackSourcePath)
{
	if (m_strTexturePackExePath.length() == 0)
	{
		return 1;
	}
	//单目录打包.
	bool bReTry = false;
	Data dDir = szPackSourcePath;
	dDir.makePath();
	dDir.formatPath();
	
	if (!VDirectory::isDirectoryExist(dDir))
	{
		return 2;
	}
	VDirectory vDir(dDir);
	Data dDirName =  vDir.getDirName(vDir.getDirNum()-1);
	char szBuffer[4096];
	
	int nRes = 0;

	while (bReTry)
	{
		bReTry = FALSE;
		Data dXMlPath = m_strOutPutDir + dDirName;
		dXMlPath += ".xml";
		Data dPNGPath = m_strOutPutDir + dDirName;
		dPNGPath += ".png";
		if (VFile::isFileExist(dXMlPath) || VFile::isFileExist(dPNGPath))
		{
			CString strWarning;
			strWarning.Format("生成目录下已经存在:%s,是否生成", dDirName.c_str());
			DWORD dwRes = AfxMessageBox(strWarning ,MB_YESNO);
			if (dwRes != IDYES)
			{
				bReTry = false;
				nRes = 6;
				continue;
			}
		}
		bReTry = false;
		char szCommandLine[1024];
		memset(szCommandLine, 0, 1024);
		memset(szBuffer, 0, 4096);
		//exe,XML,图片,目录
		sprintf_s(szCommandLine,"%s --max-size 1024 --data %s.xml --format cegui --sheet %s.png %s",m_strTexturePackExePath.c_str(), dDirName.c_str(), dDirName.c_str(),szPackSourcePath);

		SECURITY_ATTRIBUTES sa;
		HANDLE hRead(NULL), hWrite(NULL);
		//创建管道通讯.
		sa.bInheritHandle = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;
		if (!CreatePipe(&hRead,&hWrite,&sa,0))
		{
			return 3;
		}

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		si.cb = sizeof(STARTUPINFO);
		GetStartupInfo(&si);
		si.hStdError = hWrite;
		si.hStdOutput = hWrite;
		si.wShowWindow = SW_HIDE;
		si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

		BOOL bOK = CreateProcess(NULL,szCommandLine,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
		if (!bOK)
		{
			nRes = 4;
			//启动EXE失败.
			DWORD dwRes = AfxMessageBox("启动 texture pack 失败,是否重试", MB_YESNO);
			if (dwRes == IDYES)
			{
				bReTry = TRUE;
			}
			continue;

		}

		//管道获取消息.
		CloseHandle(hWrite);
		DWORD dwWaitObjRes = 0;
		std::string strRes = "";
		DWORD dwByteReaded;
		while (true)
		{
			if (ReadFile(hRead,szBuffer,4095,&dwByteReaded,NULL) == NULL)
			{
				break;
			}
			strRes += szBuffer;
		}
		CloseHandle(hRead);
		//Res  查看是否有错误.
		if (strRes.find("TexturePacker:: error:") != string::npos)
		{
			
			if (strRes.find("Not all sprites could be packed into the texture")!= string::npos)
			{
				nRes = 11;//超出1024大小.(需要拆分文件.)
				return nRes;
			}
			nRes = 5;
			//有错误发送.
			AfxMessageBox(strRes.c_str());
			DWORD dwRes = AfxMessageBox("是否重试", MB_YESNO);
			if (dwRes == IDYES)
			{
				bReTry = TRUE;
			}

			continue;
		}
		nRes = 0; //成功.

	}

	return nRes;
	
}

/*
 *	1.对比旧的XML文件，将新增的文件移动到新的文件夹.
 *  2.不存在旧的可以对比的XML文件，则估算图片大小进行拆分.
 */
int CImgPack::img_pack_split(Data dToSplit)
{
	if ( m_pXmlCur == NULL)
	{
		return -1;
	}
	dToSplit.makePath();
	dToSplit.formatPath();
	VDirectory vd(dToSplit);
	std::vector<Data> vecFiles;
	vd.getFilesVec(&vecFiles);
	if (vecFiles.size() == 0 )
	{
		return -2;
	}
	if (m_pXmlLast != NULL )
	{
		//情况1.
		Data dCurSetName = CImgToolComm::GetSignleInstance()->getSetName(dToSplit.c_str()).c_str();
		if (dCurSetName.length() == 0)
		{
			LogOut(MAIN,LOG_WARN,"CImgPack img_pack_split dir: %s no find imgset name .",dToSplit.c_str());
			return -3;
		}
		const std::map<string, _stImgSetInfo>&  mapInfo = m_pXmlLast->getMapInfo();
		std::map<string, _stImgSetInfo>::const_iterator itOld = mapInfo.find(dCurSetName.c_str());
		if (itOld != mapInfo.end())
		{
			//比较2个图集图片,找出多出的图片.
			std::vector<Data> addMap;
			int nRes  = img_cmp_set_add(itOld->second, vecFiles, addMap);
			if (nRes < 0)
			{
				//文件名重复?
				return -4;
			}
			if (nRes > 0)
			{
				//==0进入情况2.
				Data dNewSplitDir = "";
				int nRes = CSyncHandle::split_dir(addMap, m_strMainResDir+ dToSplit,dNewSplitDir);
				if (nRes == addMap.size())
				{
					return 0;
				}
				else
				{
					LogOut(MAIN, LOG_WARN, "CImgPack img_pack_split dir: %s, split error .",dToSplit.c_str());
					return -5;
				}
			}
		}
		else
		{
			LogOut(MAIN, LOG_WARN, "CImgPack img_pack_split dir: %s no find imgset name :%s,in last xml.", dToSplit.c_str(), dCurSetName.c_str());
			//旧版本不存在改图集，进入情况2
		}
	}
	//情况2.
	std::vector<Data> toSplitFile;
	int nCount = img_pack_calc(m_strMainResDir+dToSplit, vecFiles, toSplitFile);

	return 0;
}

struct  _stImgCalcInfo
{
	_stImgCalcInfo()
	{
		nWidth = 0;
		nHeigth = 0;
		nImgIndex = 0;
		bInclude = false;
	}
	int nWidth;
	int nHeigth;
	int nImgIndex;
	bool bInclude;
};

int CImgPack::img_pack_calc(Data dAbsDir, std::vector<Data> vecFiles, OUT std::vector<Data>& vecResFiles)
{
	int nRes = 0;
	std::vector<_stImgCalcInfo> vecStInfo;

	std::map<int, std::vector<int>> mapWidth;
	std::map<int, std::vector<int>> mapHeigth;

	for (int nIndex = 0; nIndex < vecFiles.size();++nIndex)
	{
		Data dImgAbsPath = dAbsDir + vecFiles[nIndex];
		if (!VFile::isFileExist(dImgAbsPath))
		{
			LogOut(MAIN, LOG_WARN, "img_pack_calc invlid file %s",dImgAbsPath.c_str());
			continue;
		}
		CXimgWrap ximg;

		if(ximg.Load(dImgAbsPath.c_str()))
		{
			_stImgCalcInfo st;
			st.nHeigth = ximg.GetHeight();
			st.nWidth = ximg.GetWidth();
			st.nImgIndex = nIndex;
			vecStInfo.push_back(st);
			mapWidth[st.nWidth].push_back(vecStInfo.size() - 1);
			mapHeigth[st.nHeigth].push_back(vecStInfo.size() - 1);
		}
		else
		{
			LogOut(MAIN, LOG_WARN, "img_pack_calc unlaw file %s", dImgAbsPath.c_str());
			continue;
		}
	}
	//开始拼图片.
	int  w[1024] = { 0 };
	int  h[1024] = { 0 };

	int nCurMaxWidth = 0;
	int nCurMaxHeigth = 0;

	std::map<int, std::vector<int>>::iterator widthIter = mapWidth.begin();
	for (widthIter; widthIter != mapWidth.end();++widthIter)
	{
		std::vector<int> & vecInt = widthIter->second;
		for (int nImgIndex = 0; nImgIndex < widthIter->second.size();++nImgIndex)
		{
			_stImgCalcInfo & stInfo = vecStInfo[vecInt[nImgIndex]];




		}
	}











	return nRes;
}

int CImgPack::img_cmp_set_add(const _stImgSetInfo &imgSetA, _stImgSetInfo& imgSetB, std::vector<Data>& vecRes)
{
	//
	vecRes.clear();
	const  IMGSETMAP &imgMapA = imgSetA.imgMap;
	IMGSETMAP &imgMapB = imgSetB.imgMap;

	IMGSETMAP::iterator itMapB = imgMapB.begin();
	for (itMapB; itMapB != imgMapB.end();++itMapB)
	{
		if (imgMapA.find(itMapB->first) == imgMapA.end())
		{
			//新增.
			vecRes.push_back(itMapB->first);
		}
	}
	return vecRes.size();
}

int CImgPack::img_cmp_set_add(const _stImgSetInfo &imgSetA, std::vector<Data>& imgFileB,std::vector<Data>& vecRes)
{
	vecRes.clear();
	const IMGSETMAP &imgMapA = imgSetA.imgMap;

	std::vector<Data>::iterator itMapB = imgFileB.begin();
	for (itMapB; itMapB != imgFileB.end(); ++itMapB)
	{
		//去掉后缀名
		Data str = itMapB->getExtName();
		Data dName = *itMapB;
/*		dName.lowerCase();*/
		dName.replace(str, "");
		if (imgMapA.find(dName.c_str()) == imgMapA.end())
		{
			//新增.
			if (std::find(vecRes.begin(), vecRes.end(),dName)!= vecRes.end())
			{
				//重名文件.
				LogOut(MAIN, LOG_ERR, "文件名重复 %s", dName.c_str());
				vecRes.clear();
				return -1;
			}
			vecRes.push_back(dName);
		}
	}
	return vecRes.size();
}

void CImgPack::setTexturepackExePath(const char* szExePath)
{
	if (!szExePath)
		return;
	m_strTexturePackExePath = szExePath;
	m_strTexturePackExePath.formatPath();
	if (!VFile::isFileExist(m_strTexturePackExePath))
	{
		m_strTexturePackExePath = "";
	}
}

void CImgPack::setOutPutDir(const char* szOutPutDir)
{
	if (!szOutPutDir)
		return;
	m_strOutPutDir = szOutPutDir;
	m_strOutPutDir.makePath();
	m_strOutPutDir.formatPath();
}



bool emun_file_callback(const Data& filename, const Data& filepath, const Data& relaDir, void* pParam)
{
	if (!pParam)
	{
		return false;
	}
	CImgPack* lpImgPack = (CImgPack*)pParam;
	return true;
}

bool enum_dir_callback(const Data& folderPath, const Data& folder, void* pParam)
{
	if (!pParam)
	{
		return false;
	}

	VDirectory vd(folderPath);
	int nMyDirNumber = vd.getDirNum();

	CImgPack* lpImgPack = (CImgPack*)pParam;
	int nCurDeep = nMyDirNumber - lpImgPack->get_work_dir_num();
	if (nCurDeep >0 && nCurDeep >= lpImgPack->m_nDeep )
	{
		//超出目录层级
		return true;
	}

	Data dFloderPath = folderPath;

	dFloderPath.formatPath();

	DWORD dwRes = lpImgPack->imp_pack(dFloderPath.c_str());
	if (dwRes == 11)
	{
		//需要拆分文件.
		
	}

}

bool CImgPack::startPack()
{
	if (!m_bChildDir)
	{
		VDirectory::EnumAllFunc(m_strMainResDir, m_strOutPutDir, NULL, enum_dir_callback, this);
	}
	else
	{
		
	}


	return false;
}
