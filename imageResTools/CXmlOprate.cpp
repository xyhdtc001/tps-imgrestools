#include "stdafx.h"
#include "CXmlOprate.h"

using namespace tinyxml2;


CXmlOprate::CXmlOprate()
{
	m_pDoc = NULL;
	m_strFilePath = "";
}

CXmlOprate::~CXmlOprate()
{
	bool bRes = true;
	if (m_pDoc != NULL)
	{
		m_pDoc->Clear();
		delete m_pDoc;;
		m_pDoc= NULL;
	}
}

bool CXmlOprate::InitXmlFile(string strFile)
{
	bool bRes = true;
	if (m_pDoc != NULL)
	{
		m_pDoc->Clear();
		delete m_pDoc;;
		m_pDoc= NULL;
	}
	m_pDoc = new XMLDocument;
	if (m_pDoc->LoadFile(strFile.c_str())!=XML_SUCCESS)
	{
		delete m_pDoc;
		m_pDoc=NULL;
		return false;
	}
	m_strFilePath = strFile;
	return bRes;
}


bool CXmlOprate::CreateXmlDoc(string strRootNode)
{
	if (m_pDoc != NULL)
	{
		m_pDoc->Clear();
		delete m_pDoc;;
		m_pDoc = NULL;
	}
	m_pDoc = new XMLDocument;
	XMLElement *pRoot = CreateEle(strRootNode);
	m_pDoc->InsertFirstChild(pRoot);
	return true;
}

bool CXmlOprate::SaveFile(string strFilePath)
{
	if (m_pDoc==NULL)
	{
		return false;
	}
	if (strFilePath.size() == 0 && m_strFilePath.size() != 0)
	{
		strFilePath = m_strFilePath ;
	}
	m_pDoc->SaveFile(strFilePath.c_str());
	return true;
}

XMLNode* CXmlOprate::GetNodePtr(string strNodeName/*=""*/)
{
	if (m_pDoc == NULL)
	{
		return NULL;
	}
	if (strNodeName.length() ==  0)
	{
		return m_pDoc->RootElement();
	}
	return m_pDoc->RootElement()->FirstChildElement(strNodeName.c_str());
}

XMLElement * CXmlOprate::CreateEle(string strname)
{
	if (m_pDoc == NULL)
	{
		return NULL;
	}
	return m_pDoc->NewElement(strname.c_str());
}
