#pragma once
#ifndef _CXMOPRATE_H
#define _CXMOPRATE_H
#include <string>

using namespace std;
using namespace tinyxml2;

#define XMLDocument tinyxml2::XMLDocument
#define XMLNode tinyxml2::XMLNode
#define  XMLElement tinyxml2::XMLElement
class CXmlOprate 
{
public:
	CXmlOprate();
	~CXmlOprate();

	virtual bool InitXmlFile(string strFile);

	virtual bool CreateXmlDoc();

	virtual bool SaveFile(string strFilePath);

	virtual XMLNode* GetNodePtr(string strNodeName="");


	virtual XMLElement * CreateEle(string strname);
protected:
	XMLDocument * m_pDoc ;

};

#endif