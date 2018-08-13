#include "stdafx.h"
#include "ImgResXmlOpreate.h"
#include "img_tools_common.h"
#include "basecode/directory.h"


CImgResXmlOpreate::CImgResXmlOpreate()
{
	m_dClientDir = "";
	m_pLastXml = NULL;
	m_strOutDir = "";
}


CImgResXmlOpreate::~CImgResXmlOpreate()
{
}

int CImgResXmlOpreate::joinXmlFile(const char* szXmlExPath, std::map<string, _stImgSetInfo>* pSetInf)
{
	int nRes = 0;
	Data dxmlFiles = szXmlExPath;
	std::vector<Data> vecFiles;
	dxmlFiles.split("|", vecFiles);


	std::map<string, _stImgSetInfo> tempMapInfo;
	for (int nIdex = 0; nIdex < vecFiles.size();++nIdex)
	{
		nRes = get_tp_xml_info(tempMapInfo, vecFiles[nIdex].c_str());
		if (nRes <= 0)
		{
			log_out(MAIN, LOG_ERR, "get tp xml info error .%s", szXmlExPath);
			return nRes;
		}
	}

	if (pSetInf)
	{
		*pSetInf = tempMapInfo;
	}
	//合并进imgres.
	 nRes = cmp_imgRes_xmlFileSet(tempMapInfo, true,false);
	 if (nRes > 0)
	 {
		 do 
		 {
			 std::map<string, _stImgSetInfo>::iterator itSetMap = tempMapInfo.begin();
			 while (itSetMap != tempMapInfo.end())
			 {
				 string strSetname = itSetMap->first;

				 //拷贝图片到目录.
				 Data dPicFile = m_strOutDir + itSetMap->second.strName;
				 dPicFile += ".png";
				 Data dDestFile = m_mapImgInfo[strSetname].strFilePath;
				 dDestFile = m_dClientDir + dDestFile;
				 if (!VFile::isFileExist(dPicFile))
				 {
					 nRes = -1;
					 log_out(MAIN, LOG_ERR, "pic file is not finded .%s", dPicFile.c_str());
					 break;;
				 }
				 else
				 {
					 //备份文件.
					 if (VFile::isFileExist(dDestFile))
					 {
						 Data dBackFile = m_strOutDir + "bak_img/";
						 dBackFile = dBackFile + m_mapImgInfo[strSetname].strFilePath;;
						 VDirectory::createFileDir(dBackFile);
						 
						 if (!VFile::copyFile(dDestFile, dBackFile, true))
						 {
							 DWORD dwError = GetLastError();
							 log_out(MAIN, LOG_ERR, "pic file is bak failed  .%s,%d", dDestFile.c_str(), dwError);
							 nRes = -2;
							 break;;
						 }
					 }
					 VDirectory::createFileDir(dDestFile);
					 
					 if (!VFile::copyFile(dPicFile, dDestFile, true))
					 {
						 DWORD dwError = GetLastError();
						 log_out(MAIN, LOG_ERR, "pic file move failed  .%s,%d", dPicFile.c_str(), dwError);
						 nRes = -2;
						 break;;
					 }
				 }
				 ++itSetMap;
			 }
		 } while (0);

	 }
	return nRes;
}

void CImgResXmlOpreate::set_work_dir(const char* szClientPath)
{
	if (szClientPath)
	{
		m_dClientDir = szClientPath;
		m_dClientDir.makePath();
		m_dClientDir.formatPath();
		Data resFile = m_dClientDir + FILE_IMGRESXML_STR;
		if (VFile :: isFileExist(resFile))
		{
			InitXmlFile(resFile.c_str());
		}
	}
	else
	{
		m_dClientDir = "";
	}
}

void CImgResXmlOpreate::set_tp_out_dir(const char * szOutDir)
{
	if(!szOutDir)
	{
		m_strOutDir = "";
	}
	else
	{
		m_strOutDir = szOutDir;
		m_strOutDir.makePath();
		m_strOutDir.formatPath();
	}
}

bool CImgResXmlOpreate::SaveFile(string strFilePath)
{
	if (strFilePath.size() == 0)
	{
		strFilePath = m_strFilePath;
	}
	return saveFileByData(m_mapImgInfo, m_strFilePath, m_pDoc);
	/*
	if (!m_pDoc )
	{
		m_pDoc = new XMLDocument;
	}
	m_pDoc->Clear();
	XMLElement *pRoot =  m_pDoc->NewElement("ImagesetSet");
	if (!pRoot)
	{
		log_out(MAIN, LOG_ERR, "CImgResXmlOpreate::SaveFile new El error");
		return false;
	}
	m_pDoc->InsertFirstChild(pRoot);
	//
	std::map<string, _stImgSetInfo>::iterator itMap = m_mapImgInfo.begin();
	while (itMap != m_mapImgInfo.end())
	{
		XMLElement *pSetNode =  m_pDoc->NewElement("Imageset");
		pRoot->InsertEndChild(pSetNode);
		pSetNode->SetAttribute("AutoScaled",itMap->second.bAutoScal);
		pSetNode->SetAttribute("Name", itMap->first.c_str());
		pSetNode->SetAttribute("Imagefile", itMap->second.strFilePath.c_str());
		if (itMap->second.nWidth > 0)
		{
			pSetNode->SetAttribute("NativeHorzRes", itMap->second.nWidth);
		}

		if (itMap->second.nHeigth > 0)
		{
			pSetNode->SetAttribute("NativeVertRes", itMap->second.nHeigth);
		}

		IMGSETMAP::iterator imgIter = itMap->second.imgMap.begin();
		while (imgIter != itMap->second.imgMap.end())
		{
			XMLElement * pImgNode = m_pDoc->NewElement("Image");
			pSetNode->InsertEndChild(pImgNode);
			pImgNode->SetAttribute("Width", imgIter->second.nWidth);
			pImgNode->SetAttribute("Height", imgIter->second.nHeigth);
			pImgNode->SetAttribute("Name", imgIter->second.strName.c_str());
			pImgNode->SetAttribute("XPos", imgIter->second.nPosX);
			pImgNode->SetAttribute("YPos", imgIter->second.nPosY);
			++imgIter;
		}
		++itMap;
	}
	return __super::SaveFile(strFilePath);
	*/
}

void CImgResXmlOpreate::update_img_info()
{
	std::map<string, _stImgSetInfo>::iterator itMap = m_mapImgInfo.begin();
	while (itMap != m_mapImgInfo.end())
	{
		IMGSETMAP::iterator imgIter = itMap->second.imgMap.begin();
		while (imgIter != itMap->second.imgMap.end())
		{
			bool bIs = CImgToolComm::GetSignleInstance()->is_3g_res(itMap->first, imgIter->first);
			if (bIs)
			{
				m_map3GInfo[imgIter->first].push_back(imgIter->second);
			}
			m_mapAllImgageInfo[imgIter->first].push_back(imgIter->second);

			++imgIter;
		}
		++itMap;
	}
}


bool CImgResXmlOpreate::get_3g_info(string strName, _stImgInfo& stInfo,string strSet)
{
	bool bRes = false;
	if (m_map3GInfo.find(strName) != m_map3GInfo.end())
	{
		bRes = true;
		for (int nIndex = 0; nIndex < m_map3GInfo[strName].size();++nIndex)
		{
			stInfo  = m_map3GInfo[strName][nIndex];
			if (strSet == stInfo.strSetName)
			{
				return bRes;
			}
		}
		return bRes;
	}
	return bRes;
}

int CImgResXmlOpreate::get_tp_xml_info(std::map<string, _stImgSetInfo>& pSetInf, const char* szXmlExPath)
{
	//TP生成的图片.xml 添加到imgres中.
	if (!szXmlExPath)
	{
		return -1;
	}
	Data dFilePath = m_strOutDir + szXmlExPath;
	Data dXmlFile = dFilePath + ".xml";
	CXmlOprate xmlOp;
	xmlOp.InitXmlFile(dXmlFile.c_str());
	XMLNode * pNodeTojoin = xmlOp.GetNodePtr();
	if (!pNodeTojoin)
	{
		return -2;
	}
	XMLElement *pEl = pNodeTojoin->ToElement();
	if (!pEl || !pEl->Attribute("Name") || !pEl->Attribute("Imagefile") || !pEl->Attribute("NativeHorzRes") || !pEl->Attribute("NativeVertRes") || !pEl->Attribute("AutoScaled"))
	{
		return -3;
	}

	std::map<string, _stImgSetInfo>& tempMapInfo = pSetInf;

	std::string strSetname = pEl->Attribute("Name");
	// common data 进行转换 看是否有更改name
	strSetname = CImgToolComm::GetSignleInstance()->getSetName(strSetname);
	if (tempMapInfo.find(strSetname) != tempMapInfo.end())
	{
		//重复
		log_out(MAIN, LOG_ERR, "get_tp_xml_info repeat set .%s", strSetname.c_str());
		return 0;
	}
	tempMapInfo[strSetname].bAutoScal = pEl->BoolAttribute("AutoScaled", false);
	tempMapInfo[strSetname].strName = strSetname;
	tempMapInfo[strSetname].strFilePath = pEl->Attribute("Imagefile");// m_mapImgInfo[strSetname].strFilePath; //为空？？(后面再处理.)
	tempMapInfo[strSetname].nWidth = pEl->IntAttribute("NativeHorzRes", 1024);
	tempMapInfo[strSetname].nHeigth = pEl->IntAttribute("NativeVertRes", 1024);
	IMGSETMAP& imgSet = tempMapInfo[strSetname].imgMap;
	XMLElement *pElChild = pEl->FirstChildElement("Image");
	while (pElChild)
	{
		do
		{
			if (!pElChild->Attribute("Name") || !pElChild->Attribute("XPos") || !pElChild->Attribute("YPos") || !pElChild->Attribute("Width")\
				|| !pElChild->Attribute("Height"))
			{
				break;
			}
			_stImgInfo  stImgInfo;
			stImgInfo.strName = pElChild->Attribute("Name");
			stImgInfo.nHeigth = pElChild->IntAttribute("Height", 0);
			stImgInfo.nWidth = pElChild->IntAttribute("Width", 0);
			stImgInfo.nPosX = pElChild->IntAttribute("XPos", 0);
			stImgInfo.nPosY = pElChild->IntAttribute("YPos", 0);
			stImgInfo.strSetName = strSetname;
			imgSet[stImgInfo.strName] = stImgInfo;
			int n3gRes = handle_3g_res(strSetname.c_str(), stImgInfo, imgSet);
		} while (0);
		pElChild = pElChild->NextSiblingElement("Image");
	}

	return 1;
}


bool CImgResXmlOpreate::saveFileByData(std::map<string, _stImgSetInfo>& pSetInf, string strFilePath, XMLDocument *pDocIn)
{
	if (strFilePath.size() == 0)
	{
		return false;
	}
	XMLDocument* pDoc;
	if (pDocIn != NULL)
	{
		pDoc = pDocIn;
	}
	else
	{
		pDoc = new XMLDocument;
	}
	pDoc->Clear();
	XMLElement *pRoot = pDoc->NewElement("ImagesetSet");
	if (!pRoot)
	{
		log_out(MAIN, LOG_ERR, "CImgResXmlOpreate::SaveFile new El error");
		pDoc->Clear();
		delete pDoc;
		return false;
	}
	pDoc->InsertFirstChild(pRoot);
	//
	std::map<string, _stImgSetInfo>::iterator itMap = pSetInf.begin();
	while (itMap != pSetInf.end())
	{
		XMLElement *pSetNode = pDoc->NewElement("Imageset");
		pRoot->InsertEndChild(pSetNode);
		pSetNode->SetAttribute("AutoScaled", itMap->second.bAutoScal);
		pSetNode->SetAttribute("Name", itMap->first.c_str());
		pSetNode->SetAttribute("Imagefile", itMap->second.strFilePath.c_str());
		if (itMap->second.nWidth > 0)
		{
			pSetNode->SetAttribute("NativeHorzRes", itMap->second.nWidth);
		}

		if (itMap->second.nHeigth > 0)
		{
			pSetNode->SetAttribute("NativeVertRes", itMap->second.nHeigth);
		}

		IMGSETMAP::iterator imgIter = itMap->second.imgMap.begin();
		while (imgIter != itMap->second.imgMap.end())
		{
			XMLElement * pImgNode = pDoc->NewElement("Image");
			pSetNode->InsertEndChild(pImgNode);
			pImgNode->SetAttribute("Width", imgIter->second.nWidth);
			pImgNode->SetAttribute("Height", imgIter->second.nHeigth);
			pImgNode->SetAttribute("Name", imgIter->second.strName.c_str());
			pImgNode->SetAttribute("XPos", imgIter->second.nPosX);
			pImgNode->SetAttribute("YPos", imgIter->second.nPosY);
			++imgIter;
		}
		++itMap;
	}
	pDoc->SaveFile(strFilePath.c_str());
	if (!pDocIn)
	{
		pDoc->Clear();
		delete pDoc;
		pDoc = NULL;
	}
	return true;
}

int CImgResXmlOpreate::handle_3g_res(const char* szSetName, _stImgInfo& imgInfo, IMGSETMAP& imgMap)
{
	IMGSETMAP mapres;
	mapres.clear();
	if (!szSetName || imgInfo.strName.size() == 0 || imgInfo.nWidth <=0 || imgInfo.nHeigth <=0 )
	{
		return 0;
	}
	//判断名字是否含有3G。
	if (!CImgToolComm::GetSignleInstance()->is_3g_res(szSetName, imgInfo.strName))
	{
		return 0;
	}
	bool bIsVertical = CImgToolComm::GetSignleInstance()->is_vertical_3g_res(imgInfo.strName);

	float nf = (float)imgInfo.nHeigth /(float)imgInfo.nWidth;
	if (nf > 3)
	{
		bIsVertical = true;
	}
	Data s3GAllName = imgInfo.strName.c_str();
	s3GAllName.lowerCase();


	string strExt1 = "_l";
	string strExt2 = "_c";
	string strExt3 = "_r";
	if (bIsVertical)
	{
		strExt1 = "_t";
		strExt2 = "_c";
		strExt3 = "_b";
	}

	//是3g图片拆分.
	if (m_pLastXml)
	{
		do 
		{
			_stImgInfo imgOrg;
			if (m_pLastXml->get_3g_info(s3GAllName.c_str(),imgOrg, szSetName))
			{
				if (imgOrg.nWidth != imgInfo.nWidth || imgOrg.nHeigth != imgInfo.nHeigth)
				{
					break;
				}
			}
			//最左边.
			_stImgInfo imgLeft;
			_stImgInfo imgMid;
			_stImgInfo imgRight;
			if (!m_pLastXml->get_3g_info(imgInfo.strName + strExt1.c_str(), imgLeft, szSetName) || !m_pLastXml->get_3g_info(imgInfo.strName + strExt2.c_str(), imgMid, szSetName) \
				|| !m_pLastXml->get_3g_info(imgInfo.strName + strExt3.c_str(), imgRight, szSetName))
			{
				break;
			}
			//取值 。
			imgLeft.nPosX = imgInfo.nPosX;
			imgLeft.nPosY = imgInfo.nPosY;
			imgMap[imgLeft.strName] = imgLeft;

			imgMid.nPosX = imgInfo.nPosX + imgLeft.nWidth;
			imgMid.nPosY = imgInfo.nPosY;
			if (bIsVertical)
			{
				imgMid.nPosX = imgInfo.nPosX;
				imgMid.nPosY = imgInfo.nPosY + imgLeft.nHeigth;
			}
			imgMap[imgMid.strName] = imgMid;


			imgRight.nPosX = imgInfo.nPosX + imgLeft.nWidth + imgMid.nWidth;
			imgRight.nPosY = imgInfo.nPosY;
			if (bIsVertical)
			{
				imgMid.nPosX = imgInfo.nPosX;
				imgMid.nPosY = imgInfo.nPosY + imgLeft.nHeigth + imgMid.nHeigth;
			}
			imgMap[imgRight.strName] = imgRight;

			return 1;

		} while (0);

	}

	//自动拆分.
	_stImgInfo stTemp = imgInfo;
	stTemp.nPosX = imgInfo.nPosX;
	stTemp.nPosY = imgInfo.nPosY;
	int nWidth = (float)imgInfo.nWidth*0.3;
	int nHeigth = (float)imgInfo.nHeigth*0.3;;
	stTemp.nWidth = nWidth;
	stTemp.nHeigth = imgInfo.nHeigth;
	stTemp.strName = imgInfo.strName+strExt1.c_str();
	if (bIsVertical)
	{
		stTemp.nWidth = imgInfo.nWidth;
		stTemp.nHeigth = nHeigth ;
	}
	imgMap[imgInfo.strName+ strExt1.c_str()] = stTemp;

	stTemp = imgInfo;
	stTemp.nPosX = imgInfo.nPosX + nWidth;
	stTemp.nPosY = imgInfo.nPosY;
	stTemp.nWidth = imgInfo.nWidth - nWidth * 2;
	stTemp.strName = imgInfo.strName + strExt2.c_str();
	if (bIsVertical)
	{
		stTemp.nPosX = imgInfo.nPosX;
		stTemp.nPosY = imgInfo.nPosY + nHeigth;
		stTemp.nWidth = imgInfo.nWidth;
		stTemp.nHeigth = imgInfo.nHeigth - nHeigth * 2;
	}
	imgMap[imgInfo.strName + strExt2.c_str()] = stTemp;

	stTemp = imgInfo;
	stTemp.nPosX = imgInfo.nPosX + imgInfo.nWidth - nWidth ;
	stTemp.nPosY = imgInfo.nPosY;
	stTemp.nWidth = nWidth;
	stTemp.strName = imgInfo.strName + strExt3.c_str();
	if (bIsVertical)
	{
		stTemp.nPosX = imgInfo.nPosX;
		stTemp.nPosY = imgInfo.nPosY + imgInfo.nHeigth -  nHeigth;
		stTemp.nWidth = imgInfo.nWidth;
		stTemp.nHeigth = nHeigth ;
	}
	imgMap[imgInfo.strName + strExt3.c_str()] = stTemp;
	return 1;
}
