#include "stdafx.h"
#include "img_pack.h"


#include "basecode/VFile.h"
#include "basecode/directory.h"
#include "imgresxml_cmp.h"
#include "img_tools_common.h"
#include "sync_handle.h"

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
	//��Ŀ¼���.
	bool bReTry = true;
	Data dDir = szPackSourcePath;
	dDir.makePath();
	dDir.formatPath();
	
	if (!VDirectory::isDirectoryExist(dDir))
	{
		return 2;
	}

	std::vector<Data>vecPathSpit;
	dDir.split("/", vecPathSpit);
	if (vecPathSpit.size()==0)
	{
		return 2;
	}
	Data dDirName = vecPathSpit[vecPathSpit.size() - 1];
	char szBuffer[4096];
	
	int nRes = 0;

	while (bReTry)
	{
		bReTry = false;
		Data dXMlPath = m_strOutPutDir + dDirName;
		dXMlPath += ".xml";
		Data dPNGPath = m_strOutPutDir + dDirName;
		dPNGPath += ".png";
		if (VFile::isFileExist(dXMlPath) || VFile::isFileExist(dPNGPath))
		{
			CString strWarning;
			strWarning.Format("����Ŀ¼���Ѿ�����:%s,�Ƿ�����", dDirName.c_str());
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
		//exe,XML,ͼƬ,Ŀ¼
		dDirName = m_strOutPutDir + dDirName;
		sprintf_s(szCommandLine,"%s --max-size 1024 --data %s.xml --format cegui --sheet %s.png %s",m_strTexturePackExePath.c_str(), dDirName.c_str(), dDirName.c_str(),szPackSourcePath);

		SECURITY_ATTRIBUTES sa;
		HANDLE hRead(NULL), hWrite(NULL);
		//�����ܵ�ͨѶ.
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
			//����EXEʧ��.
			DWORD dwRes = AfxMessageBox("���� texture pack ʧ��,�Ƿ�����", MB_YESNO);
			if (dwRes == IDYES)
			{
				bReTry = TRUE;
			}
			continue;

		}

		//�ܵ���ȡ��Ϣ.
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
		//Res  �鿴�Ƿ��д���.
		if (strRes.find("TexturePacker:: error:") != string::npos)
		{
			
			if (strRes.find("Not all sprites could be packed into the texture")!= string::npos)
			{
				nRes = 11;//����1024��С.(��Ҫ����ļ�.)
				return nRes;
			}
			nRes = 5;
			//�д�����.
			AfxMessageBox(strRes.c_str());
			DWORD dwRes = AfxMessageBox("�Ƿ�����", MB_YESNO);
			if (dwRes == IDYES)
			{
				bReTry = TRUE;
			}

			continue;
		}
		nRes = 0; //�ɹ�.
		m_curVecOutPut.push_back(dDirName);
	}

	return nRes;
	
}

/*
 *	1.�ԱȾɵ�XML�ļ������������ļ��ƶ����µ��ļ���.
 *  2.�����ھɵĿ��ԶԱȵ�XML�ļ��������ͼƬ��С���в��.
 */
int CImgPack::img_pack_split(Data dToSplit,Data& newDir)
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
		//���1.
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
			//�Ƚ�2��ͼ��ͼƬ,�ҳ������ͼƬ.
			std::vector<Data> addMap;
			int nRes  = img_cmp_set_add(itOld->second, vecFiles, addMap);
			if (nRes < 0)
			{
				//�ļ����ظ�?
				return -4;
			}
			if (nRes > 0)
			{
				//==0�������2.
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
			//�ɰ汾�����ڸ�ͼ�����������2
		}
	}
	//���2.
	std::vector<Data> toSplitFile;
	int nCount = img_pack_calc(m_strMainResDir+dToSplit, vecFiles, toSplitFile);
	if (nCount > 0)
	{
		//����ļ���.
		Data dDirNewExport = "";
		int nRes = CSyncHandle::split_dir(toSplitFile, m_strMainResDir + dToSplit, dDirNewExport);
		if (nRes != toSplitFile.size())
		{
			LogOut(MAIN, LOG_ERR, "splitDir Fail ");
			return -6;
		}
		newDir = dDirNewExport;
	}
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
		xPos = 0;
		yPos = 0;
	}
	int nWidth;
	int nHeigth;
	int nImgIndex;
	bool bInclude;
	int xPos;
	int yPos;
};
/************************************************************************/
/* ƴͼƬ�㷨                                                           */
/************************************************************************/


/*
 *	���Ͻǿ�ʼѰ���Ƿ��д�С���ʵ�����.
 */
int find_rect_area(int* szPic,CSize szRect,bool bSetRect,int nMaxLen,CPoint &ptLT)
{
	int nResX = -1;
	int nResY = -1;
	//��ƥ����.
	bool bOk = false;
	int nCurLineX = 0;
	int nCurLineY = 0;
	for (int nIndexX = 0; nIndexX < nMaxLen; ++nIndexX)
	{
		for (int nIndexY = 0; nIndexY < nMaxLen;++ nIndexY)
		{
			if (nIndexX + szRect.cx >=nMaxLen)
			{
				nCurLineX = 0;
				continue;
			}
			int nCurPos = nIndexX + nIndexY*nMaxLen;
			int &nPointValue = *(szPic + nCurPos);
			if (nPointValue > 0)
			{
				nCurLineX = 0;
				continue;
			}
			++nCurLineX;
			bool bVLineOk = true;
			//ƥ��߶��Ƿ��㹻.
			for (int nIndexEx = 0; nIndexEx < szRect.cy; ++nIndexEx)
			{
				if (nIndexEx + nIndexY >= nMaxLen)
				{
					bVLineOk = false;
					break;
				}
				int &nPointValue = *(szPic + nCurPos + nIndexEx*nMaxLen);
				if (nPointValue>0)
				{
					bVLineOk = false;
					break;
				}
			}
			if (bVLineOk == false)
			{
				nCurLineX = 0;
				continue;
			}
			if(nCurLineX == szRect.cx)
			{ 
				nResX = nIndexX;
				nResY = nIndexY;
				bOk = true;
				break;
			}
		}
	}
	ptLT.x = nResX;
	ptLT.y = nResY;
	if (nResY != -1 && nResX != -1)
	{
		if (bSetRect)
		{
			// ��ֵ.
			for (int nIndex = nResY; nIndex < szRect.cy;++nIndex)
			{
				memset(szPic+nResX+nMaxLen*nIndex,1,szRect.cx);
			}
		}
		return 0;
	}
	return -1;
}

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
		CImage img;
		if(img.Load(dImgAbsPath.c_str()))
		{
			_stImgCalcInfo st;
			st.nHeigth = img.GetHeight();
			st.nWidth = img.GetWidth();
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
	//��ʼƴͼƬ.
	int  picData[1024*1024] = { 0 };


	std::map<int, std::vector<int>>::iterator widthIter = mapWidth.begin();
	for (widthIter; widthIter != mapWidth.end();++widthIter)
	{
		std::vector<int> & vecInt = widthIter->second;
		for (int nImgIndex = 0; nImgIndex < widthIter->second.size();++nImgIndex)
		{
			_stImgCalcInfo & stInfo = vecStInfo[vecInt[nImgIndex]];
			//Ѱ���㹻��С����.
			CPoint pt;
			DWORD dwTime = GetTickCount();
			int nRes = find_rect_area(picData,CSize(stInfo.nWidth,stInfo.nHeigth),true,1024,pt);
			dwTime = GetTickCount() - dwTime;
			LogOut(MAIN,LOG_WARN,"pic process time %d,dir:%s,res:%d",dwTime,dAbsDir.c_str(),nRes);
			if (nRes >= 0)
			{
				stInfo.bInclude = true;
				stInfo.xPos = pt.x;
				stInfo.yPos = pt.y;
			}
			else
			{
				vecResFiles.push_back(vecFiles[stInfo.nImgIndex]);
			}
		}
	}
	return vecResFiles.size();
}


/************************************************************************/
/* ƴͼƬ�㷨                                          END              */
/************************************************************************/




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
			//����.
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
		//ȥ����׺��
		Data str = itMapB->getExtName();
		Data dName = *itMapB;
/*		dName.lowerCase();*/
		dName.replace(str, "");
		if (imgMapA.find(dName.c_str()) == imgMapA.end())
		{
			//����.
			if (std::find(vecRes.begin(), vecRes.end(),dName)!= vecRes.end())
			{
				//�����ļ�.
				LogOut(MAIN, LOG_ERR, "�ļ����ظ� %s", dName.c_str());
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




void CImgPack::setXmlPtr(CImgresXmlCmp *lpXmlCur, CImgresXmlCmp* lpXmlLast)
{
	if (lpXmlLast)
	{
		m_pXmlCur = lpXmlCur;
	}
	if (lpXmlLast)
	{
		lpXmlLast = lpXmlLast;
	}
}

bool CImgPack::PackSignDir(const Data& folderPath)
{

	Data dFloderPath = folderPath;

	dFloderPath.formatPath();

	DWORD dwRes = imp_pack(dFloderPath.c_str());
	if (dwRes == 11)
	{
		//��Ҫ����ļ�.
		Data dNewDir;
		int nRes = img_pack_split(dFloderPath, dNewDir);
		if (nRes < 0 )
		{
			LogOut(MAIN,LOG_ERR,"enum_dir_callback split error ,nRes:%d",nRes);
			return false;
		}
		//����ɨ�������ļ�.
		if (PackSignDir(dFloderPath))
		{
			return PackSignDir(dNewDir);
		}
		else
		{
			return false;
		}
	}
	LogOut(MAIN,LOG_WARN,"enum_dir_callback %s ,res:%d",dFloderPath.c_str(),dwRes);
	return dwRes == 0;
}

bool CImgPack::startPack(bool bChildDir)
{
	m_curVecOutPut.clear();
	bool bRes = true;
	m_bChildDir = bChildDir;
	VDirectory vd(m_strMainResDir);

	if (m_bChildDir)
	{
		vd.getSubDirs();

		std::vector<Data> vecDir = vd.getDirVec();
		for (int nIndex = 0; nIndex < vecDir.size();++nIndex)
		{
			if (vecDir[nIndex] == "new_t_single")
			{
				continue;
			}
			if (!PackSignDir(m_strMainResDir + vecDir[nIndex]))
			{
				bRes = false;
				break;
			}
		}
	}
	else
	{
		bRes = PackSignDir(m_strMainResDir);
	}
	return bRes;
}
