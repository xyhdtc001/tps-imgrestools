#include "stdafx.h"
#include "ImgResXmlOpreate.h"
#include "img_tools_common.h"


CImgResXmlOpreate::CImgResXmlOpreate()
{
	m_dClientDir = "";
	m_pLastXml = NULL;
	m_strOutDir = "";
}


CImgResXmlOpreate::~CImgResXmlOpreate()
{
}

int CImgResXmlOpreate::joinXmlFile(const char* szXmlExPath)
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
	if (!pEl || pEl->Attribute("Name") || pEl->Attribute("Imagefile") || pEl->Attribute("NativeHorzRes") || pEl->Attribute("NativeVertRes") || pEl->Attribute("AutoScaled"))
	{
		return -3;
	}

	std::map<string, _stImgSetInfo> tempMapInfo;

	std::string strSetname = pEl->Attribute("Name");
	// common data 进行转换 看是否有更改name
	strSetname = CImgToolComm::GetSignleInstance()->getSetName(strSetname);

	if (m_mapImgInfo.find(strSetname) == m_mapImgInfo.end())
	{
		//全部添加.
		if (diffMapAdd.find(strSetname) != diffMapAdd.end())
		{
			// 重复添加.
			CString strError;
			strError.Format("%s 重复添加,是否继续", strSetname.c_str());
			int nRes = AfxMessageBox(strError,MB_YESNO);
			if (nRes == IDNO)
			{
				return -5;
			}
		}
	}

	tempMapInfo[strSetname].bAutoScal = pEl->BoolAttribute("AutoScaled",false);
	tempMapInfo[strSetname].strName = strSetname;
	tempMapInfo[strSetname].strFilePath = "";// m_mapImgInfo[strSetname].strFilePath; //为空？？(后面再处理.)
	tempMapInfo[strSetname].nWidth = pEl->IntAttribute("NativeHorzRes", 1024);
	tempMapInfo[strSetname].nHeigth = pEl->IntAttribute("NativeVertRes", 1024);
	IMGSETMAP& imgSet = tempMapInfo[strSetname].imgMap;
	XMLElement *pElChild =  pEl->FirstChildElement("Image");
	while (pElChild)
	{
		do 
		{
			if (!pElChild->Attribute("Name")|| !pElChild->Attribute("XPos")|| !pElChild->Attribute("YPos")|| !pElChild->Attribute("Width")\
				|| !pElChild->Attribute("Height"))
			{
				break;
			}
			_stImgInfo  stImgInfo;
			stImgInfo.strName = pElChild->Attribute("Name");
			stImgInfo.nHeigth = pElChild->IntAttribute("Height",0);
			stImgInfo.nWidth = pElChild->IntAttribute("Width", 0);
			stImgInfo.nPosX = pElChild->IntAttribute("XPos",0);
			stImgInfo.nPosY = pElChild->IntAttribute("YPos", 0);
			imgSet[stImgInfo.strName] = stImgInfo;
			int n3gRes = handle_3g_res(strSetname.c_str(), stImgInfo, imgSet);
		} while (0);
		pElChild = pElChild->NextSiblingElement("Image");
	}

	//合并进imgres.
	 int nRes = cmp_imgRes_xmlFileSet(tempMapInfo, true,false);
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

	Data s3GAllName = imgInfo.strName.c_str();
	s3GAllName.lowerCase();
	s3GAllName += "_3g";

	//是3g图片拆分.
	if (m_pLastXml)
	{
		do 
		{
			const std::map<string, _stImgSetInfo> & lastMapInfo = m_pLastXml->getMapInfo();

			if (lastMapInfo.find(szSetName) != lastMapInfo.end())
			{
				const IMGSETMAP& setInfo = lastMapInfo.find(szSetName)->second.imgMap;

				//检查3G图片大小是否有更改 。
				IMGSETMAP::const_iterator itAll3gIter = setInfo.find(s3GAllName.c_str());
				if (itAll3gIter != setInfo.end())
				{
					if (itAll3gIter->second.nWidth != imgInfo.nWidth || itAll3gIter->second.nHeigth != imgInfo.nHeigth)
					{
						break;
					}
				}
				//最左边.
				IMGSETMAP::const_iterator itLeft = setInfo.find(imgInfo.strName);
				IMGSETMAP::const_iterator itMidd = setInfo.find(imgInfo.strName+"_2");
				IMGSETMAP::const_iterator itRight = setInfo.find(imgInfo.strName+"_3");
				if (itLeft== setInfo.end() || itMidd == setInfo.end() || itRight == setInfo.end())
				{
					break;
				}
				//取值 。
				_stImgInfo stTemp = itLeft->second;
				stTemp.nPosX = imgInfo.nPosX;
				stTemp.nPosY = imgInfo.nPosY;
				imgMap.insert(std::make_pair(itLeft->first, stTemp));

				stTemp = itMidd->second;
				stTemp.nPosX = imgInfo.nPosX + itLeft->second.nWidth;
				stTemp.nPosY = imgInfo.nPosY ;
				imgMap.insert(std::make_pair(itLeft->first, stTemp));


				stTemp = itRight->second;
				stTemp.nPosX = imgInfo.nPosX  +itLeft->second.nWidth + itMidd->second.nWidth;
				stTemp.nPosY = imgInfo.nPosY;
				imgMap.insert(std::make_pair(itLeft->first, stTemp));

				stTemp = imgInfo;
				stTemp.nPosX = imgInfo.nPosX ;
				stTemp.nPosY = imgInfo.nPosY;
				imgMap.insert(std::make_pair(s3GAllName.c_str(), stTemp));

				return 1;
			}

		} while (0);

	}

	//自动拆分.
	_stImgInfo stTemp = imgInfo;
	stTemp.nPosX = imgInfo.nPosX;
	stTemp.nPosY = imgInfo.nPosY;
	int nWidth = (float)imgInfo.nWidth*0.3;
	stTemp.nWidth = nWidth;
	stTemp.nHeigth = imgInfo.nHeigth;
	imgMap.insert(std::make_pair(imgInfo.strName, stTemp));

	stTemp = imgInfo;
	stTemp.nPosX = imgInfo.nPosX + nWidth;
	stTemp.nPosY = imgInfo.nPosY;
	stTemp.nWidth = imgInfo.nWidth - nWidth * 2;
	imgMap.insert(std::make_pair(imgInfo.strName+"_2", stTemp));


	stTemp = imgInfo;
	stTemp.nPosX = imgInfo.nPosX + imgInfo.nWidth - nWidth * 2;
	stTemp.nPosY = imgInfo.nPosY;
	stTemp.nWidth = nWidth;
	imgMap.insert(std::make_pair(imgInfo.strName+"_3", stTemp));

	stTemp = imgInfo;
	imgMap.insert(std::make_pair(s3GAllName.c_str(), stTemp));

	return 1;
}
