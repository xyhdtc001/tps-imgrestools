#include "stdafx.h"
#include "img_tools_common.h"
#include "CXmlOprate.h"

CImgToolComm * CImgToolComm::m_signleInstance = NULL;

CImgToolComm::CImgToolComm()
{
	m_MapDirToName.clear();
	m_Map3gRes.clear();
}

CImgToolComm::~CImgToolComm()
{
}

CImgToolComm* CImgToolComm::GetSignleInstance()
{
	if (!CImgToolComm::m_signleInstance)
	{
		CImgToolComm::m_signleInstance = new CImgToolComm;
	}
	return CImgToolComm::m_signleInstance;
}

void CImgToolComm::setWorkDir(LPCSTR szWorkDir)
{
	if (szWorkDir)
	{
		m_workDir = szWorkDir;
		m_workDir.makePath();
		init_map_dirtoname();
	}
}

void CImgToolComm::init_map_dirtoname()
{
	Data filePath = m_workDir+ "common_set_dirtoname.xml";
	CXmlOprate xmlOp;
	xmlOp.InitXmlFile(m_workDir.c_str());
	if (!xmlOp.GetNodePtr())
	{
		LogOut(MAIN,LOG_ERR,"faild open %s,xml init failed ." ,filePath.c_str());
		return;
	}
	m_MapDirToName.clear();
	XMLNode* pRoot = xmlOp.GetNodePtr() ;
	XMLNode *pChildNode = pRoot->FirstChild();
	while (pChildNode)
	{
		do 
		{
			XMLElement *pEl = pChildNode->ToElement();
			if (!pEl)
			{
				break;
			}
			if (!pEl->Attribute("setname") || !pEl->Attribute("dirname"))
			{
				break;
			}
			Data strDirName = pEl->Attribute("dirname");
			strDirName.lowerCase();
			Data strSetName = pEl->Attribute("setname");
			m_MapDirToName[strDirName].strName = strDirName.c_str();

		} while (0);

		pChildNode = pChildNode->NextSibling();
	}


	
}

void CImgToolComm::init_map_3gres()
{

}

string CImgToolComm::getSetName(string strDirName)
{
	string strRes = strDirName;
	Data dKey = strDirName.c_str();
	dKey.lowerCase();
	if (m_MapDirToName.find(dKey) != m_MapDirToName.end())
	{
		return m_MapDirToName[dKey].strName;
	}
	return strRes;
}

bool CImgToolComm::is_3g_res(string strSetname, string strImgName)
{
	Data dKey = strSetname.c_str();
	dKey += "_";
	dKey += strImgName.c_str();
	if (m_Map3gRes.find(strSetname) != m_Map3gRes.end())
	{
		return true;
	}
	
	Data dName = strImgName.c_str();
	dName.lowerCase();
	if (dName.find("_3g_")> 0)
	{
		m_Map3gRes.insert(dKey);
		return true;
	}
	return false;


}
