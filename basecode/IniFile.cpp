#include "basecode/BaseLog.h"
#include "basecode/IniFile.h"
#include "basecode/VFile.h"

IniFile::IniFile()
{

}

IniFile::IniFile( const String &fileName, bool bOverwrite, bool bCreate )
{
	openIniFile( fileName, bOverwrite, bCreate );
}

IniFile::~IniFile()
{
	close();
}


bool IniFile::openIniFile( const String& fileName, bool bOverWrite, bool bCreate )
{
	VFile	file;
	FIELDS_ITER fieldIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
		delete *fieldIter;
	m_vecField.clear();

	if ( bOverWrite )
	{
		if ( !file.openFile(fileName.c_str(),VFile::READWRITE) )
		{
			LogOut( MAIN, LOG_ERR, "Create ini file err: %s", fileName.c_str() );
			return false;
		}
	}
	else
	{	
		if ( !file.openFile(fileName.c_str(), VFile::READ) )
		{
			if ( bCreate )
			{
				if ( !file.openFile(fileName.c_str(), VFile::READWRITE) )
				{
					LogOut( MAIN, LOG_ERR, "Create ini file err: %s", fileName.c_str() );
					return false;
				}
			}
			else
			{
				LogOut( MAIN, LOG_ERR, "Open ini file err: %s", fileName.c_str() );
				return false;
			}
		}
	}
	m_dtFileName = fileName;
	load( file.getAllData().get() );
	file.closeFile();
	return true;
}

void IniFile::load( Stream* stream )
{
	String dtLine;
	IniField* pField = NULL;
	String::SizeType pos = String::NPos;
	while ( stream && stream->getStatus() != Stream::EOS )
	{
		uint8 line[4096];
		stream->readLine( line, 4096 );
		String dtLine = (char*)line;
		dtLine.removeCRLF();
		dtLine.removeSpaces();
		dtLine.removeTab();

		if ( dtLine[0]=='\\' && dtLine[1]=='\\' )
			continue;

		if ( dtLine[0]==';'  )
			continue;


		if ( dtLine[0]=='/' && dtLine[1]=='/' )
			continue;

		if ( dtLine[0] == '[' && (pos=dtLine.find("]"))!=String::NPos )
		{
			if ( pField != NULL )
				m_vecField.push_back( pField );
			String dtName = dtLine.substr( 1,pos-1 );
			dtName.removeSpaces();
			dtName.removeTab();
			pField = new IniField( dtName );
			continue;
		}
		if ( pField == NULL )
			continue;
		if ( (pos=dtLine.find("=")) != String::NPos )
		{
			String name = dtLine.substr( 0, pos );
			String val = dtLine.substr( pos+1 );
			name.removeCRLF();
			name.removeSpaces();
			name.removeTab();
			if ( (pos=val.find ("\\\\")) != String::NPos )
				val = val.substr( 0, pos );
			/*
			if ( (pos=val.find ("//")) != String::NPos )
				val = val.substr( 0, pos );
				*/
			val.removeCRLF();
			val.removeSpaces();
			val.removeTab();
			pField->addItem( IniItem(name,val) );
		}
		else
		{
			dtLine.removeCRLF();
			dtLine.removeSpaces();
			dtLine.removeTab();
			if ( (pos=dtLine.find ("\\\\")) != -1 )
				dtLine = dtLine.substr( 0, pos );
			/*
			if ( (pos=dtLine.find ("//")) != -1 )
				dtLine = dtLine.substr( 0, pos );
				*/
			pField->addItem( IniItem(dtLine,"") );
		}
	}
	if ( pField != NULL )
		m_vecField.push_back( pField );
}

uint32 IniFile::getFieldNum()
{
	return (uint32)m_vecField.size();
}

uint32 IniFile::getItemNum( const String& fieldName )
{
	FIELDS_ITER fieldIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if( fieldIter != m_vecField.end() )
	{
		return (uint32)(*fieldIter)->m_ItemList.size();
	}
	else
	{
		return 0;
	}
}


uint32 IniFile::getItemNum( unsigned int fieldIndex )
{
	if ( fieldIndex >= m_vecField.size() )
		return 0;
	return (uint32)m_vecField[fieldIndex]->m_ItemList.size();
}

void IniFile::setFieldName( String fieldName, unsigned int fieldIndex )
{
	if ( fieldIndex >= m_vecField.size() )
		return;
	m_vecField[fieldIndex]->m_strName = fieldName;
}

String IniFile::getFieldName( unsigned int fieldIndex )
{
	if ( fieldIndex >= m_vecField.size() )
		return String();
	return m_vecField[fieldIndex]->m_strName;
}


int	IniFile::getFieldIndex( const String& fieldName )
{
	int index = -1;
	int i = 0;
	for ( FIELDS_ITER iter=m_vecField.begin(); iter!=m_vecField.end(); iter++ )
	{
		if ( (*iter)->m_strName.equal(fieldName,String::NoCase) )
		{
			index = i;
			break;
		}
		i++;
	}
	return index;
}


IniField* IniFile::getFieldByName( const String& fieldName )
{
	int i = 0;
	for ( FIELDS_ITER iter=m_vecField.begin(); iter!=m_vecField.end(); iter++ )
	{
		if ( (*iter)->m_strName.equal(fieldName,String::NoCase) )
		{
			return (*iter);
		}
		i++;
	}
	return NULL;
}


IniField* IniFile::getFieldByIndex( unsigned int index )
{
	return m_vecField[index];
}

void IniFile::removeField( const String& fieldName )
{
	for ( FIELDS_ITER iter=m_vecField.begin(); iter!=m_vecField.end(); iter++ )
	{
		if ( (*iter)->m_strName.equal(fieldName,String::NoCase) )
		{
			delete (*iter);
			m_vecField.erase( iter );
			return;
		}
	}	
}


void IniFile::removeItem( const String& fieldName, const String& itemName )
{
	FIELDS_ITER fieldIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
		{
			break;
		}
	}
	if ( fieldIter == m_vecField.end() )
	{
		return;
	}
	else
	{
		ITEMS_ITER itemIter;
		for ( itemIter=(*fieldIter)->m_ItemList.begin(); itemIter!=(*fieldIter)->m_ItemList.end(); itemIter++ )
		{
			if ( itemIter->m_strName.equal(itemName,String::NoCase) )
				break;
		}
		if ( itemIter != (*fieldIter)->m_ItemList.end() )
		{
			(*fieldIter)->m_ItemList.erase(itemIter);
		}
	}
}


String IniFile::getItemName( const String& fieldName, unsigned int itemIndex )
{
	FIELDS_ITER fieldIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
	{
		return String();
	}
	else
	{
		if ( itemIndex >= (*fieldIter)->m_ItemList.size() )
			return String();
		return (*fieldIter)->m_ItemList[itemIndex].m_strName;
	}
}


void IniFile::setItemStr( const String& fieldName, const String& itemName, const String& itemVal )
{
	ITEMS_ITER itemIter;
	FIELDS_ITER fieldIter;
	IniItem item( itemName, itemVal );
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
	{
		IniField* pField = new IniField;
		pField->m_strName = fieldName;
		pField->addItem( item );
		m_vecField.push_back( pField );
	}
	else
	{
		IniField* pField = *fieldIter;
		for ( itemIter=pField->m_ItemList.begin(); itemIter!=pField->m_ItemList.end(); itemIter++ )
		{
			if ( itemIter->m_strName.equal(itemName,String::NoCase) )
				break;
		}
		if ( itemIter == (*fieldIter)->m_ItemList.end() )
		{
			pField->addItem( item );
		}
		else
		{
			itemIter->m_strValue = itemVal;
		}
	}
}


String IniFile::getItemName( unsigned int fieldIndex, unsigned int itemIndex )
{
	if ( fieldIndex >= m_vecField.size() )
	{
		return String();
	}
	if ( itemIndex >= m_vecField[fieldIndex]->m_ItemList.size() )
		return String();
	ITEMS_ITER itemIter = m_vecField[fieldIndex]->m_ItemList.begin();
	for ( unsigned int i=0; i<itemIndex; i++ )
	{
		itemIter++;
	}
	return itemIter->m_strName;
}


String IniFile::getItemStr( const String& fieldName, const String& itemName )
{
	FIELDS_ITER fieldIter;
	ITEMS_ITER	itemIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
	{
		return String();
	}
	else
	{
		return (*fieldIter)->getItemStr( itemName );
	}
}

String IniFile::getItemStrDef( const String& fieldName, const String& itemName, const char* defVal )
{
	FIELDS_ITER fieldIter;
	ITEMS_ITER	itemIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
	{
		return defVal;
	}
	else
	{
		return (*fieldIter)->getItemStrDef( itemName, defVal );
	}
}

String IniFile::getItemStr( unsigned int fieldIndex, const String& itemName )
{
	ITEMS_ITER	itemIter;
	if ( fieldIndex >= m_vecField.size() )
	{
		return String();
	}
	IniField* pField = &( *m_vecField[fieldIndex] );
	return pField->getItemStr( itemName );
}

String IniFile::getItemStr( unsigned int fieldIndex, unsigned int itemIndex )
{
	if ( fieldIndex >= m_vecField.size() || itemIndex>=m_vecField[fieldIndex]->m_ItemList.size() )
	{
		return String();
	}
	return m_vecField[fieldIndex]->m_ItemList[itemIndex].m_strValue.c_str();
}


String IniFile::getItemStr( const String& fieldName, unsigned int itemIndex )
{
	FIELDS_ITER fieldIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
	{
		return String();
	}
	else
	{
		if ( itemIndex > (*fieldIter)->m_ItemList.size() )
		{
			return String();
		}
		return (*fieldIter)->m_ItemList[itemIndex].m_strValue;
	}	
}


bool IniFile::hasItem( const String& fieldName, const String& itemName )
{
	FIELDS_ITER fieldIter;
	ITEMS_ITER	itemIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
	{
		return false;
	}
	
	for ( itemIter=(*fieldIter)->m_ItemList.begin(); itemIter!=(*fieldIter)->m_ItemList.end(); itemIter++ )
	{
		if ( itemIter->m_strName.equal(itemName,String::NoCase) )
		{
			return true;
		}
	}
	return false;
}


int IniFile::getItemIndex( const String& fieldName, const String& itemName )
{
	FIELDS_ITER fieldIter;
	ITEMS_ITER	itemIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		if ( (*fieldIter)->m_strName.equal(fieldName,String::NoCase) )
			break;
	}
	if ( fieldIter == m_vecField.end() )
		return -1;
	int index = 0;
	IniField* pField = *fieldIter;
	for ( itemIter=pField->m_ItemList.begin(); itemIter!=pField->m_ItemList.end(); itemIter++ )
	{
		if ( itemIter->m_strName.equal(itemName,String::NoCase) )
			break;
		index++;
	}
	if ( itemIter == pField->m_ItemList.end() )
		return -1;
	else
		return index;
}


bool IniFile::saveIniFile()
{
	String dtEq = "=";
	if ( m_dtFileName.isEmpty() )
		return false;

	VFile file;	
	if ( !file.openFile(m_dtFileName.c_str(), VFile::READWRITE) )
		return false;
	FIELDS_ITER fieldIter;
	ITEMS_ITER itemIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		file.write( 1, (unsigned char*)"[" ); 
		file.write( (*fieldIter)->m_strName.c_str() );
		file.write( 1, (unsigned char*)"]" );
		file.write( CRLF );
		for ( itemIter = (*fieldIter)->m_ItemList.begin(); itemIter!=(*fieldIter)->m_ItemList.end(); itemIter++ )
		{
			file.write( itemIter->m_strName.c_str() );
			file.write( dtEq.c_str() );
			file.write( itemIter->m_strValue.c_str() );
			file.write( CRLF );
		}
		file.write( CRLF );
	}
	file.closeFile();
	return true;
}


void IniFile::close()
{
	m_dtFileName.clear();
	FIELDS_ITER fieldIter;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
		delete *fieldIter;
	m_vecField.clear();

}

unsigned  IniFile::getAllItemNum()
{
	FIELDS_ITER fieldIter;
	uint32 num = 0;
	for ( fieldIter=m_vecField.begin(); fieldIter!=m_vecField.end(); fieldIter++ )
	{
		num += (uint32)(*fieldIter)->m_ItemList.size();
	}
	return num;
}
