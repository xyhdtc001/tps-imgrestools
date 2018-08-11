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
				m_mapImgInfo[strName].strName = strName;

				XMLElement *pImage = pImgset->FirstChildElement("Image");
				while (pImage)
				{
					if (pImage->Attribute("Width") && pImage->Attribute("Height") && pImage->Attribute("Name"))
					{
						string strImgName = pImage->Attribute("Name");
						imgsetMap[strImgName].strName = strImgName;
						imgsetMap[strImgName].nWidth = pImage->IntAttribute("Width");
						imgsetMap[strImgName].nHeigth = pImage->IntAttribute("Height");
						imgsetMap[strImgName].nPosX = pImage->IntAttribute("XPos");
						imgsetMap[strImgName].nPosY = pImage->IntAttribute("YPos");
						imgsetMap[strImgName].strSetName = strImgName;
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
		return -1;
	}
	CImgresXmlCmp xmlEx;
	xmlEx.InitXmlFile(szXmlExPath);
	if (!xmlEx.GetNodePtr())
	{
		return -1;
	}
	//比较2 个XML的map

	int nRes = cmp_imgRes_xmlFileSet(xmlEx.getMapInfo(),false,bImgSetCmp);
	return nRes;
}

int CImgresXmlCmp::cmp_imgRes_xmlFileSet(const std::map<string, _stImgSetInfo>& xmlEx, bool bJoin ,bool bImgSetCmp)
{
	std::map<string, _stImgSetInfo>::iterator itOrg = m_mapImgInfo.begin();
	std::vector<string>vecFindSet;
	while (itOrg != m_mapImgInfo.end())
	{
		do
		{
			if (xmlEx.find(itOrg->first) == xmlEx.end())
			{
				if (bImgSetCmp)
				{
					diffMapDee[itOrg->first] = itOrg->second;
					if (bJoin)
					{
						itOrg->second = _stImgSetInfo();
					}
				}
				break;
			}
			string strFilePath = itOrg->second.strFilePath;
			vecFindSet.push_back(itOrg->first);
			std::map<string, _stImgSetInfo>::const_iterator itExMap = xmlEx.find(itOrg->first);
			diffMapAdd[itOrg->first].bAutoScal = itExMap->second.bAutoScal;
			diffMapAdd[itOrg->first].bOwner = itExMap->second.bOwner;
			diffMapAdd[itOrg->first].nWidth = itExMap->second.nWidth;
			diffMapAdd[itOrg->first].nHeigth = itExMap->second.nHeigth;
			diffMapAdd[itOrg->first].strName = itExMap->second.strName;
			diffMapAdd[itOrg->first].strFilePath = strFilePath;

			

			//初始化.
			diffMapDee[itOrg->first] = diffMapAdd[itOrg->first];

			IMGSETMAP& imgSet = itOrg->second.imgMap;
			const IMGSETMAP& imgSetEx = itExMap->second.imgMap;
			IMGSETMAP& imgSetAdd = diffMapAdd[itOrg->first].imgMap;
			IMGSETMAP& imgSetDee = diffMapDee[itOrg->first].imgMap;
			IMGSETMAP& imgSetModify = diffMapModify[itOrg->first].imgMap;
			IMGSETMAP::iterator imgIter = imgSet.begin();
			std::vector<string>vecFind;
			vecFind.clear();
			while (imgIter != imgSet.end())
			{
				if (imgSetEx.find(imgIter->first) == imgSetEx.end())
				{
					//缺失.
					imgSetDee[imgIter->first] = imgIter->second;
				}
				else
				{
					IMGSETMAP::const_iterator imgIterEx = imgSetEx.find(imgIter->first);
					vecFind.push_back(imgIter->first);
					if (imgIterEx->second.nWidth != imgIter->second.nWidth || \
						imgIterEx->second.nHeigth != imgIter->second.nHeigth)
					{
						imgSetModify[imgIter->first] = imgIterEx->second;
					}
				}
				++imgIter;
			}

			IMGSETMAP::const_iterator imgIterEx = imgSetEx.begin();
			//新增的图片.
			while (imgIterEx != imgSetEx.end())
			{
				if (std::find(vecFind.begin(),vecFind.end(),imgIterEx->first) == vecFind.end())
				{
					imgSetAdd[imgIterEx->first] = imgIterEx->second;
				}
				++imgIterEx;
			}

			//替换新数据.
			if (bJoin)
			{
				itOrg->second = itExMap->second;
				itOrg->second.strFilePath = strFilePath;
			}
		} while (0);

		++itOrg;
	}

	//新增的节点.
	std::map<string, _stImgSetInfo>::const_iterator itExSet = xmlEx.begin();
	while (itExSet != xmlEx.end())
	{
		if (std::find(vecFindSet.begin(), vecFindSet.end(), itExSet->first) == vecFindSet.end())
		{
			std::string strDef = "data/ui/imageset_comm/";
			strDef = strDef + itExSet->second.strFilePath;

			diffMapAdd[itExSet->first] = itExSet->second;
			diffMapAdd[itExSet->first].strFilePath = strDef;
			//替换新数据.
			if (bJoin)
			{
				m_mapImgInfo[itExSet->first] = itExSet->second;
				m_mapImgInfo[itExSet->first].strFilePath = strDef;
			}
		}
		++itExSet;
	}
	return 1;
}
