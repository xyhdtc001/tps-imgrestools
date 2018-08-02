#pragma once

#include <sys/types.h>
#include <stdlib.h>
#include <list>
#include <vector>
#include <string.h>
#include <algorithm>

#include "basecode/str.h"
#include "basecode/stream.h"

using namespace std;

class IniItem
{
public:
	IniItem( const IniItem& item )
	{
		m_strName = item.m_strName;
		m_strValue = item.m_strValue;
		m_strMemo = item.m_strMemo;
	}

	IniItem( const String& strName, const String& strValue, const String& strMemo="" )
	{
		m_strName = strName;
		m_strValue = strValue;
		m_strMemo = strMemo;
	}

	IniItem( const String& strName )
	{
		m_strName = strName;
	}

	virtual ~IniItem()
	{
	}

public:	
	friend bool operator==( const IniItem& item1, const IniItem& item2 )
	{
		return ( item1.m_strName == item2.m_strName );
	}

	friend bool operator==( const IniItem& item, const String& dtName )
	{
		return ( item.m_strName == dtName );
	}

	friend bool operator!=( const IniItem& item, const String& dtName )
	{
		return ( item.m_strName != dtName );
	}

public:
	String m_strName;			//项目的名字
	String m_strValue;		//项目的字符内容
	String m_strMemo;
};

typedef vector<IniItem> ITEMS;
typedef vector<IniItem>::iterator ITEMS_ITER;
class IniField
{
public:
	IniField(){};

	IniField( const IniField& field )
	{
		m_strName = field.m_strName;
		m_ItemList = field.m_ItemList;
	}

	String getName()
	{
		return m_strName;
	}

	IniField( String& strName )
	{
		m_strName = strName;
	}

	virtual ~IniField()
	{
	}

	unsigned int getItemNum()
	{
		return (uint32)m_ItemList.size();
	}

	void addItem( const IniItem& item )
	{
		m_ItemList.push_back( item );
	}

	String getItemNameByIndex( unsigned int index )
	{
		if ( index >= m_ItemList.size() )
			return String();
		return m_ItemList[index].m_strName.c_str();
	}

	String getItemStr( const String& name )
	{
		ITEMS_ITER	itemIter;
		for ( itemIter=m_ItemList.begin(); itemIter!=m_ItemList.end(); itemIter++ )
		{
			if ( itemIter->m_strName.equal(name,String::NoCase) )
			{
				return itemIter->m_strValue.c_str();
			}
		}
		return String();
	}

	String getItemStrDef( const String& name, const char* defStr )
	{
		ITEMS_ITER	itemIter;
		for ( itemIter=m_ItemList.begin(); itemIter!=m_ItemList.end(); itemIter++ )
		{
			if ( itemIter->m_strName.equal(name,String::NoCase) )
			{
				return itemIter->m_strValue.c_str();
			}
		}
		return String(defStr);
	}

	String getItemStr( unsigned int index )
	{
		if ( index >= m_ItemList.size() )
			return String();
		return m_ItemList[index].m_strValue;
	}

	int getItemInt( const String& name )
	{
		return getItemStr(name).toInt32();
	}

	String getItemValueByIndex( unsigned index )
	{
		if ( index >= m_ItemList.size() )
			return String();
		return m_ItemList[index].m_strValue;
	}

	friend bool operator==( const IniField& field1, const IniField& field2 )
	{
		return ( field1.m_strName == field2.m_strName );
	}

	friend bool operator==( const IniField& field1, const char* strName )
	{
		return ( field1.m_strName == String(strName) );
	}

	friend bool operator!=( const IniField& field1, const char* strName )
	{
		return ( field1.m_strName != strName );
	}

public:
	String	m_strName;
	list< String > m_dtMemoList;
	ITEMS	m_ItemList;
};


typedef vector<IniField*> FIELDS;
typedef vector<IniField*>::iterator FIELDS_ITER;
class IniFile  
{
public:
	String	getItemStr( unsigned int fieldIndex, const String& itemName  );
	String	getItemStr( const String& fieldName, const String& itemName  );
	String	getItemStrDef( const String& fieldName, const String& itemName, const char* defVal );

	String	getItemStr( unsigned int fieldIndex, unsigned int itemIndex );
	String	getItemStr( const String& fieldName, unsigned int itemIndex );

	void	setItemStr( const String& fieldName, const String& itemName, const String& itemVal );

	String	getItemName( unsigned int fieldIndex, unsigned int itemIndex );
	String	getItemName( const String&, unsigned int itemIndex );

	int		getItemIndex( const String& fieldName, const String& itemName );

	void    setFieldName( String fieldName, unsigned int fieldIndex );
	String	getFieldName( unsigned int fieldIndex );
	int		getFieldIndex( const String& fieldName );
	IniField* getFieldByName( const String& fieldName );
	IniField* getFieldByIndex( unsigned int index );

	bool	hasItem( const String& fieldName, const String& itemName );

	void	removeField( const String& fieldName );
	void	removeItem( const String& fieldName, const String& itemName );

	uint32 getItemNum( unsigned int fieldIndex );
	uint32 getItemNum( const String& fieldName );

	unsigned int getAllItemNum();

	bool	hasField();

	uint32	getFieldNum();
	bool	openIniFile( const String& fileName, bool bOverWrite=false, bool bCreate=false );
	void	load( Stream* stream );
	bool	saveIniFile();
	void	close();

	IniFile();
	IniFile( const String& fileName, bool bOverwrite = false, bool bCreate=false );

	virtual ~IniFile();
	void	enableEncFile( unsigned char* key );	

private:
	String			m_dtFileName;
	FIELDS			m_vecField;					//域列表
	unsigned char	m_encKey[8];
	bool			m_bEncFile;
};
