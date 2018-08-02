#ifndef _CLIPBOARD_H_
#define _CLIPBOARD_H_

#include "basecode/basefunc.h"
#include "basecode/basefunc_metro.h"

#ifdef WINDOWS
#ifdef METRO
#else
HWND g_hWnd = NULL;
BOOL CALLBACK EnumWindowProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessID = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessID);

	if (dwProcessID != (DWORD)lParam)
		return TRUE;

	if (GetParent(hWnd) == NULL)
	{
		g_hWnd = hWnd;
		return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK EnumChildWindowProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessID = 0;
	GetWindowThreadProcessId(hWnd, &dwProcessID);

	if (dwProcessID != GetCurrentProcessId())
		return TRUE;

	if (GetWindowLongPtr(hWnd, GWLP_HINSTANCE) == lParam)
	{
		g_hWnd = hWnd;
		return FALSE;
	}

	return TRUE;
}
#endif
#endif

inline void initClipboard()
{
#ifdef WINDOWS
#ifdef METRO
#else
	char buf[MAX_PATH];
	GetModuleFileName(0, (LPCH)&buf, MAX_PATH);
	HINSTANCE instance = GetModuleHandle(buf);
	EnumChildWindows(GetDesktopWindow(), (WNDENUMPROC)EnumWindowProc, (LPARAM)instance);
#endif
#endif
}

inline void setClipboardTextData(const String& _clipdata)
{
#ifdef WINDOWS
#ifdef METRO
	using namespace Windows::ApplicationModel::DataTransfer;
	Platform::String^ buff = str2wstr(_clipdata.c_str(), _clipdata.length());

	DataPackage^ dataPackage = ref new DataPackage;
	dataPackage->SetText(buff);
	Clipboard::SetContent(dataPackage);

#else
	if (OpenClipboard((HWND)g_hWnd))
	{
		EmptyClipboard();
		size_t size = (_clipdata.numChars() + 1) * sizeof(UTF16);
		HGLOBAL hgBuffer = GlobalAlloc(GMEM_DDESHARE, size);
		UTF16* chBuffer = hgBuffer ? (UTF16*)GlobalLock(hgBuffer) : NULL;
		if (chBuffer)
		{
			memset(chBuffer, 0, size);
			size_t length = _clipdata.numChars()  * sizeof(UTF16);
			memcpy(chBuffer, _clipdata.utf16(), length);
			SetClipboardData(CF_UNICODETEXT, hgBuffer);
			GlobalUnlock(hgBuffer);
		}
		CloseClipboard();
	}
#endif
#endif
}

inline String getClipboardTextData()
{
	String buff;
#ifdef WINDOWS
#ifdef METRO
	using namespace Windows::ApplicationModel::DataTransfer;
	DataPackageView^ dataPackageView;
	dataPackageView = Clipboard::GetContent();
	if (dataPackageView->Contains(StandardDataFormats::Text))
	{
		Platform::String^ strbuff = dataPackageView->GetTextAsync()->ToString();
		buff = wstr2str(strbuff);
	}
#else
	if (OpenClipboard((HWND)g_hWnd))
	{
		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		UTF16* chBuffer = hData ? (UTF16*)GlobalLock(hData) : NULL;
		if (chBuffer)
		{
			buff = chBuffer;
			GlobalUnlock(hData);
		}
		CloseClipboard();
	}
#endif
#endif
	return buff;
}


#endif