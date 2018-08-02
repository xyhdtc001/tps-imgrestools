#include "basecode/basefunc_metro.h"

Platform::String^ str2wstr( const char* str, uint32 len )
{
	uint32 iLen = MultiByteToWideChar( CP_ACP, 0, str, len, 0, 0 );
	wchar_t* buf = new wchar_t[iLen];
	MultiByteToWideChar( CP_ACP, 0, str, iLen, buf, iLen );
	Platform::String^ strHostName = ref new Platform::String( buf );
	delete[] buf;
	return strHostName;
}

const char* wstr2str( Platform::String^ str )
{
	const wchar_t* wbuf = str->Data();
	unsigned int length = str->Length();
	int bytes = WideCharToMultiByte(0, 0, wbuf, length, NULL, 0, NULL, NULL);

	char* buf = new char[bytes];
	WideCharToMultiByte(0, 0, wbuf, length, buf, bytes, NULL, NULL);
	return buf;
}
