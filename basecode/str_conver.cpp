
#include <Windows.h>
#include "str_conver.h"

CStrConver::CStrConver(void)
{
	m_pszUnknown = new char[2];
	m_pszUnknown[0] = ' ';
	m_pszUnknown[1] = 0;
}

CStrConver::~CStrConver(void)
{
	delete[] m_pszUnknown;
	m_pszUnknown = NULL;
}

//big5 to GBK_����
LPSTR CStrConver::Big52GBKSimplified(char * szText)
{
	int nLength;
	wchar_t *pBuffer;
	LPSTR pResult;
	int nResultLength;

	nLength = MultiByteToWideChar(950, 0, szText, strlen(szText), NULL, 0);
	pBuffer = new wchar_t[nLength + 1];
	MultiByteToWideChar(950, 0, (LPCSTR)szText, strlen(szText), (LPWSTR)pBuffer, nLength);
	pBuffer[nLength] = 0;

	nResultLength = WideCharToMultiByte(936, 0, pBuffer, nLength, NULL, 0, m_pszUnknown, FALSE);
	pResult = new char[nResultLength + 1];
	WideCharToMultiByte(936, 0, (LPWSTR)pBuffer, nLength, (LPSTR)pResult, nResultLength, "  ", FALSE);
	pResult[nResultLength] = 0;

	return GBKTraditional2GBKSimplified(pResult);

}

//big5 to GBK_����
LPSTR CStrConver::Big52GBKTraditional(char * szText)
{
	int nLength;
	wchar_t *pBuffer;
	LPSTR pResult;
	int nResultLength;

	nLength = MultiByteToWideChar(950, 0, szText, strlen(szText), NULL, 0);
	pBuffer = new wchar_t[nLength + 1];
	MultiByteToWideChar(950, 0, (LPCSTR)szText, strlen(szText), (LPWSTR)pBuffer, nLength);
	pBuffer[nLength] = 0;

	nResultLength = WideCharToMultiByte(936, 0, pBuffer, nLength, NULL, 0, m_pszUnknown, FALSE);
	pResult = new char[nResultLength + 1];
	WideCharToMultiByte(936, 0, (LPWSTR)pBuffer, nLength, (LPSTR)pResult, nResultLength, "  ", FALSE);
	pResult[nResultLength] = 0;

	return pResult;
}

//GBK_���� to GBK_����
LPSTR CStrConver::GBKTraditional2GBKSimplified(char * szTraditional)
{
	LCID dwLocale;
	WORD wLangID;
	wLangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
	dwLocale = MAKELCID(wLangID, SORT_CHINESE_PRC);

	int nLength;
	char *pBuffer;
	nLength = LCMapStringA(dwLocale, LCMAP_SIMPLIFIED_CHINESE, (LPCSTR)szTraditional, strlen(szTraditional), NULL, 0);
	pBuffer = new char[nLength + 1];
	pBuffer[nLength] = 0;
	LCMapStringA(dwLocale, LCMAP_SIMPLIFIED_CHINESE, (LPCSTR)szTraditional, strlen(szTraditional), pBuffer, nLength);
	return pBuffer;
}

//GBK_���� to big5
LPSTR CStrConver::GBK2Big5(char * szText)
{
	LPSTR szGBKTraditional;
	int nLength;
	wchar_t *pBuffer;
	LPSTR pResult;
	int nResultLength;

	szGBKTraditional = GBKSimplified2GBKTraditional(szText);
	nLength = MultiByteToWideChar(936, 0, szGBKTraditional, strlen(szGBKTraditional), NULL, 0);
	pBuffer = new wchar_t[nLength + 1];
	MultiByteToWideChar(936, 0, (LPCSTR)szGBKTraditional, strlen(szGBKTraditional), (LPWSTR)pBuffer, nLength);
	pBuffer[nLength] = 0;

	nResultLength = WideCharToMultiByte(950, 0, pBuffer, nLength, NULL, 0, m_pszUnknown, FALSE);
	pResult = new char[nResultLength + 1];
	WideCharToMultiByte(950, 0, (LPWSTR)pBuffer, nLength, (LPSTR)pResult, nResultLength, "  ", FALSE);
	pResult[nResultLength] = 0;

	return pResult;
}

//��GBK�ļ���ת����GBK����
LPSTR CStrConver::GBKSimplified2GBKTraditional(char * szSimplified)
{
	LCID dwLocale;
	WORD wLangID;
	wLangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
	dwLocale = MAKELCID(wLangID, SORT_CHINESE_PRC);

	int nLength;
	char *pBuffer;
	nLength = LCMapStringA(dwLocale, LCMAP_TRADITIONAL_CHINESE, (LPCSTR)szSimplified, strlen(szSimplified), NULL, 0);
	pBuffer = new char[nLength + 1];
	pBuffer[nLength] = 0;
	LCMapStringA(dwLocale, LCMAP_TRADITIONAL_CHINESE, (LPCSTR)szSimplified, strlen(szSimplified), pBuffer, nLength);
	return pBuffer;
}

// ת����Unicode
LPWSTR CStrConver::ToUnicode(char * szSource, int nEncoding)
{
	int nLength;
	wchar_t *pBuffer;
	int nLanguage;

	if (nEncoding == CHINESE_SIMPLIFIED)
		nLanguage = 936;
	else
		if (nEncoding == CHINESE_TRADITIONAL)
			nLanguage = 950;
		else
			nLanguage = CP_ACP;

	nLength = MultiByteToWideChar(nLanguage, 0, szSource, strlen(szSource), NULL, 0);
	pBuffer = new wchar_t[nLength + 1];
	MultiByteToWideChar(nLanguage, 0, (LPCSTR)szSource, strlen(szSource), (LPWSTR)pBuffer, nLength);
	pBuffer[nLength] = 0;

	return pBuffer;
}

//ת�������ֽ�
LPSTR CStrConver::ToMultiByte(LPCWSTR szSource, int nEncoding)
{
	int nLength;
	char *pBuffer;
	int nLanguage;

	if (nEncoding == CHINESE_SIMPLIFIED)
		nLanguage = 936;
	else
		if (nEncoding == CHINESE_TRADITIONAL)
			nLanguage = 950;
		else
			nLanguage = CP_ACP;

	nLength = WideCharToMultiByte(nLanguage, 0, szSource, wcslen(szSource), NULL, 0, m_pszUnknown, FALSE);

	pBuffer = new char[nLength + 1];
	WideCharToMultiByte(nLanguage, 0, szSource, wcslen(szSource), pBuffer, nLength, m_pszUnknown, FALSE);
	pBuffer[nLength] = 0;

	return pBuffer;

}

//UTF8ת����UNICODE
LPWSTR CStrConver::UTF82UNICODE(char*   utf8str)
{
	int nLength;
	wchar_t *pBuffer;

	nLength = MultiByteToWideChar(CP_UTF8, 0, utf8str, strlen(utf8str), NULL, 0);
	pBuffer = new wchar_t[nLength + 1];
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)utf8str, strlen(utf8str), (LPWSTR)pBuffer, nLength);
	pBuffer[nLength] = 0;

	return pBuffer;
}

//UNICODEת����UTF8
LPSTR CStrConver::UNICODE2UTF8(LPCWSTR  strText)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)strText, -1, NULL, 0, NULL, NULL);
	char *szUtf8 = new char[2 * (len + 1)];
	memset(szUtf8, 0, len * 2 + 2);  //UTF8�����ֽ��������һ��UINICODE�ַ���ռ�ֽ���������
	WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)strText, -1, szUtf8, len, NULL, NULL);
	return szUtf8;

}