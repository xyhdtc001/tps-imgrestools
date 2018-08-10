#include "stdafx.h"
#include "sync_handle.h"

#include "basecode/VFile.h"
#include "basecode/directory.h"
#include "ImgResXmlOpreate.h"

CSyncHandle::CSyncHandle()
{
	m_strExtFileName.insert("png");
	m_strExtFileName.insert("jpg");
	m_strExtFileName.insert("jpeg");
}


CSyncHandle::~CSyncHandle()
{

}

void CSyncHandle::setWorkDir(const char * strWorkDir)
{
	if (!strWorkDir)
	{
		return;
	}
	m_strWorkDir = strWorkDir;
}


bool scanfiletoxml(const Data &filename, const Data& filepath, const Data& relaDir, void *pParam)
{
	_sysFileParam *pSysFileInfo = (_sysFileParam*)pParam;
	if (pSysFileInfo == NULL || !pSysFileInfo->ptr_syncHandle)
	{
		return false;
	}
	Data relPath = filepath;
	relPath.formatPath();

	Data ext = filename.getExtName();
	ext.lowerCase();
	if (pSysFileInfo->ptr_syncHandle->isHandleExt(ext.c_str()))
	{
		//扩展名过滤.
		return true;
	}

	if (pSysFileInfo->nCurScanState == 0)
	{

		relPath.replace(pSysFileInfo->dStanDir, "");
		if (relPath.length() != 0)
		{
			pSysFileInfo->mapStandFilePathInfo[filename.c_str()].setPaths.insert(relPath.c_str());
		}


	}
	else
	{
		relPath.replace(pSysFileInfo->dDesDir, "");
		if (relPath.length() > 0)
		{
			pSysFileInfo->mapTocFilePathInfo[filename.c_str()].setPaths.insert(relPath.c_str());
		}

	}
	return true;
}


bool CSyncHandle::sysDir(std::string strStandDir, std::string strToChangeDir)
{
	initAppData();

	Data dStanDir = strStandDir.c_str();
	Data dDestDir = strToChangeDir.c_str();
	dStanDir.makePath();
	dStanDir.formatPath();
	dDestDir.makePath();
	dDestDir.formatPath();

	//param
	_sysFileParam stParam;
	stParam.dStanDir = dStanDir;
	stParam.dDesDir = dDestDir;
	stParam.mapStandFilePathInfo.clear();
	stParam.mapTocFilePathInfo.clear();


	VDirectory dir;

	bool bBreak = false;
	do
	{
		stParam.nCurScanState = 0;
		//扫描标准目录.
		if (false == dir.EnumAllFunc(dStanDir, dStanDir, scanfiletoxml, NULL, &stParam))
		{
			return false;
		}
	} while (bBreak);


	do
	{
		stParam.nCurScanState = 1;
		//扫描目标目录.
		if (false == dir.EnumAllFunc(dDestDir, dDestDir, scanfiletoxml, NULL, &stParam))
		{
			return false;
		}
	} while (bBreak);

	//标准目录对比目标目录.
	std::map<string, _sysFileInfo>::iterator itStan = stParam.mapStandFilePathInfo.begin();
	while (itStan != stParam.mapStandFilePathInfo.end())
	{
		std::string strName = itStan->first;
		if (itStan->second.setPaths.size() > 1)
		{
			// 			CString strError;
			// 			strError.Format("文件%s,不同目录下重名",strName.c_str());
			// 			if (IDYES!=AfxMessageBox(strError,MB_YESNO))
			// 			{
			// 				return false;
			// 			}
		}

		if (stParam.mapTocFilePathInfo.find(strName) != stParam.mapTocFilePathInfo.end())
		{
			Data dMoveDir = "";
			_sysFileInfo &stToc = stParam.mapTocFilePathInfo[strName];
			SETSTRING::iterator itSetString = itStan->second.setPaths.begin();
			//处理文件.
			while (itSetString != itStan->second.setPaths.end())
			{
				Data dS = (*itSetString).c_str();
				dS.formatPath();
				dS.lowerCase();
				bool bSure = false;;
				SETSTRING::iterator itTocset = stToc.setPaths.begin();
				//对比待处理
				while (stToc.setPaths.end() != itTocset)
				{
					Data dD = (*itTocset).c_str();
					dD.formatPath();
					dD.lowerCase();
					if (dD == dS)
					{
						dMoveDir = "";
						break;
					}
					if (dMoveDir.length() > 0)
					{
						bSure = true;
						//逼近判断..
						Data dtemp = dMoveDir;
						if (dtemp.length() > dS.length())
						{
							dtemp.replace(dS, "");
							if (dtemp.length() < dMoveDir.length())
							{
								dMoveDir = dD;
							}
						}
						else
						{
							dtemp = dS;
							dtemp.replace(dMoveDir, "");
							if (dtemp.length() < dS.length())
							{
								dMoveDir = dD;
							}
						}
					}
					else
						dMoveDir = dD;
					++itTocset;
				}

				if (dMoveDir.length() > 0)
				{
					if (bSure)
					{
						int ii = 0;
					}
					//移动文件.
					VDirectory::createDir(stParam.dDesDir + dS);
					if (!VFile::move(stParam.dDesDir + dMoveDir + strName.c_str(), stParam.dDesDir + dS + strName.c_str(), false))
					{
						CString strError;
						strError.Format("文件%s,移动失败目录:%s.", strName.c_str(), dMoveDir.c_str());
						//错误.
						AfxMessageBox(strError);
					}
				}
				++itSetString;

			}

		}

		++itStan;

	}

	return true;

}

/*
 *	初始化程序配置数据.
 */
void CSyncHandle::initAppData()
{
	
}

bool CSyncHandle::isHandleExt(const char * szExt)
{
	if (szExt)
		return false;
	return m_strExtFileName.find(szExt) != m_strExtFileName.end();
}


/*
 *	拆分文件夹.
 */
int CSyncHandle::split_dir(std::vector<Data> strFile, Data dDir, Data& dDirNewExport)
{
	int nMoveCount = 0;
	dDir.makePath();
	dDir.formatPath();
	if (!VDirectory::isDirectoryExist(dDir))
	{
		return nMoveCount;
	}
	if (dDirNewExport.length() == 0)
	{
		dDirNewExport = dDir.substr(0,dDir.length()-1);
		//判断原来目录是否是拆分过的。
		int nNewIndex = 0;
		if (dDirNewExport.find("_sp") == dDirNewExport.length()-4 )
		{
			Data dOldIndex = dDirNewExport.substr(dDirNewExport.length()-1,1);
			nNewIndex = dOldIndex.toInt32() + 1;
		}
		dDirNewExport += "_sp";
		if (nNewIndex > 10 || nNewIndex < 1)
		{
			return nMoveCount;
		}
		char szIndex[20] = {0};
		sprintf(szIndex, "%d", nNewIndex);
		dDirNewExport += szIndex;
		dDirNewExport.makePath();
	}

	if (!VDirectory::isDirectoryExist(dDirNewExport))
	{
		//目录已经存在.
		return nMoveCount;
	}

	//移动文件.
	VDirectory::createDir(dDirNewExport);
	std::vector<Data>::iterator itVecFile = strFile.begin();
	while (itVecFile != strFile.end())
	{
		bool bRetry = false;
		bool bActionRes = false;
		do 
		{
			bRetry = false;
			if ((*itVecFile).length() <= 0)
			{
				break;
			}
			Data dFullFilePath = dDir + (*itVecFile);
			if (!VFile::isFileExist(dFullFilePath))
			{
				LogOut(MAIN,LOG_WARN,"sync_handle - split_dir - %s file is not exist .",dFullFilePath.c_str());
				break;
			}
			Data dMoveTo = dDirNewExport+(*itVecFile);
			if (!VFile::move(dFullFilePath, dMoveTo,true))
			{
				LogOut(MAIN, LOG_WARN, "sync_handle - split_dir - %s file move failed .", dFullFilePath.c_str());
				CString strInfo;
				strInfo.Format("%s 移动失败，是否重试?", dFullFilePath.c_str());
				int bRes  = AfxMessageBox(strInfo,MB_YESNO);
				if (bRes == IDYES)
				{
					bRetry = true;
				}
				break;
			}

			bActionRes = true;
			nMoveCount++;
			bRetry = false;

		} while (bRetry);

		++itVecFile;
	}
	return nMoveCount;
}
