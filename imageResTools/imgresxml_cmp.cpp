#include "stdafx.h"
#include "imgresxml_cmp.h"


CImgresXmlCmp::CImgresXmlCmp()
{
}

CImgresXmlCmp::~CImgresXmlCmp()
{
}

bool CImgresXmlCmp::InitXmlFile(string strFile)
{
	bool bres = __super::InitXmlFile(strFile);
	if (bres)
	{
		//扫描出imgageset的节点.
		XMLElement * pImgset = GetNodePtr()->FirstChildElement("Imageset");

		while (pImgset != NULL)
		{
			do 
			{
				if (!pImgset->Attribute("Name") || !pImgset->Attribute("Imagefile"))
				{
					break;
				}
				string strName = pImgset->Attribute("Name");
				if (m_mapImgInfo.find(strName) != m_mapImgInfo.end())
				{
					CString strWarning;
					strWarning.Format("重复 图片集：%s", strName.c_str());
					AfxMessageBox(strWarning);
					break;
				}
				IMGSETMAP &imgsetMap =  m_mapImgInfo[strName].imgMap;
				m_mapImgInfo[strName].bAutoScal = false;
				m_mapImgInfo[strName].bOwner = true;
				m_mapImgInfo[strName].strFilePath = pImgset->Attribute("Imagefile");

				XMLElement *pImage = pImgset->FirstChildElement("Image");
				while (pImage)
				{
					if (pImage->Attribute("Width") && pImage->Attribute("Height") && pImage->Attribute("Name"))
					{
						string strImgName = pImage->Attribute("Name");
						imgsetMap[strImgName].strName = strImgName;
						imgsetMap[strImgName].nWidth = pImage->IntAttribute("Width");
						imgsetMap[strImgName].nHeigth = pImage->Int64Attribute("Height");
					}
					pImage = pImage->NextSiblingElement("Image");
				}
			} while (0);
			pImgset = pImgset->NextSiblingElement("Imageset");
		}
	}
	return bres;
}

int CImgresXmlCmp::cmpImgresXmlFile(const char* szXmlExPath,bool bImgSetCmp)
{
	if (!m_pDoc )
	{
		return 0;
	}
	CImgresXmlCmp xmlEx;
	xmlEx.InitXmlFile(szXmlExPath);
	if (!xmlEx.GetNodePtr())
	{
		return 0;
	}
	//比较2 个XML的map

	std::map<string, _stImgSetInfo>::iterator itOrg = m_mapImgInfo.begin();
	while (itOrg != m_mapImgInfo.end())
	{
		do 
		{
			if (xmlEx.m_mapImgInfo.find(itOrg->first) == xmlEx.m_mapImgInfo.end())
			{
				if (bImgSetCmp)
				{
					diffMapDee[itOrg->first] = itOrg->second;
				}
				break;
			}
			xmlEx.m_mapImgInfo[itOrg->first].bFind = true;
			diffMapAdd[itOrg->first].bAutoScal = xmlEx.m_mapImgInfo[itOrg->first].bAutoScal;
			diffMapAdd[itOrg->first].bOwner = xmlEx.m_mapImgInfo[itOrg->first].bOwner;
			diffMapAdd[itOrg->first].nWidth = xmlEx.m_mapImgInfo[itOrg->first].nWidth;
			diffMapAdd[itOrg->first].nHeigth = xmlEx.m_mapImgInfo[itOrg->first].nHeigth;
			diffMapAdd[itOrg->first].strName = xmlEx.m_mapImgInfo[itOrg->first].strName;
			diffMapAdd[itOrg->first].strFilePath = xmlEx.m_mapImgInfo[itOrg->first].strFilePath;

			//初始化.
			diffMapDee[itOrg->first] = diffMapAdd[itOrg->first];
			
			IMGSETMAP& imgSet = itOrg->second.imgMap;
			IMGSETMAP& imgSetEx = xmlEx.m_mapImgInfo[itOrg->first].imgMap;
			IMGSETMAP& imgSetAdd = diffMapAdd[itOrg->first].imgMap;
			IMGSETMAP& imgSetDee = diffMapDee[itOrg->first].imgMap;
			IMGSETMAP::iterator imgIter = imgSet.begin();
			while (imgIter != imgSet.end())
			{
				if (imgSetEx.find(imgIter->first) == imgSetEx.end())
				{
					//缺失.
					imgSetDee[imgIter->first] = imgIter->second;
				}
				else
				{
					imgSetEx[imgIter->first].bFind = true;
				}
				++imgIter;
			}

			IMGSETMAP::iterator imgIterEx = imgSetEx.begin();
			//新增的图片.
			while (imgIterEx != imgSetEx.end())
			{
				if (imgIterEx->second.bFind == false)
				{
					imgSetAdd[imgIterEx->first] = imgIterEx->second;
				}
				++imgIterEx;
			}
		} while (0);

		++itOrg;
	}

	//新增的节点.
	std::map<string, _stImgSetInfo>::iterator itExSet = xmlEx.m_mapImgInfo.begin();
	while (itExSet != xmlEx.m_mapImgInfo.end())
	{
		if (itExSet->second.bFind == false)
		{
			diffMapDee[itExSet->first] = itExSet->second;
		}
		++itExSet;
	}
	return 1;
}
