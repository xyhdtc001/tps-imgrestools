#include "windows.h"
#include <gdiplus.h>
#include "basecode/BaseType.h"
#include "basecode/ui_func.h"
#include "basecode/basefunc.h"


IStream* initPng( LPTSTR res )
{
	HMODULE hInstance = ::GetModuleHandle(NULL);
	HRSRC hRsrc = ::FindResource( hInstance, res, _T("PNG") );
	DWORD dwSize = ::SizeofResource( hInstance, hRsrc );
	LPBYTE lpRes = (LPBYTE)::LoadResource( hInstance, hRsrc );
	HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
	LPBYTE pMem = (LPBYTE)::GlobalLock(hMem);
	memcpy( pMem, lpRes, dwSize );
	IStream* pStream = NULL;
	::CreateStreamOnHGlobal( hMem, FALSE, &pStream);
	return pStream;
}


void parsePng( CImage* pImag )
{
	unsigned char* pCol = 0;
	int32 iw = pImag->GetWidth();
	int32 ih = pImag->GetHeight();
	for ( int32 y=0; y<ih; y++ )
	{
		for ( int32 x=0; x<iw; x++ )
		{
			pCol = (unsigned char*)pImag->GetPixelAddress(x,y);
			unsigned char alpha = pCol[3];
			if ( alpha<255 )
			{
				pCol[0] = ((pCol[0]*alpha)+127) / 255;
				pCol[1] = ((pCol[1]*alpha)+127) / 255;
				pCol[2] = ((pCol[2]*alpha)+127) / 255;
			}
		}
	}
}

IStream* initJpg( LPTSTR res )
{
	HMODULE hInstance = ::GetModuleHandle(NULL);
	HRSRC hRsrc = ::FindResource( hInstance, res, _T("JPG") );
	DWORD dwSize = ::SizeofResource( hInstance, hRsrc );
	LPBYTE lpRes = (LPBYTE)::LoadResource( hInstance, hRsrc );
	HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
	LPBYTE pMem = (LPBYTE)::GlobalLock(hMem);
	memcpy( pMem, lpRes, dwSize );
	IStream* pStream = NULL;
	::CreateStreamOnHGlobal( hMem, FALSE, &pStream );
	return pStream;
}

HRGN getRghFromPng( uint32 id, RECT* pRect )
{

	HRGN hRgn = CreateRectRgn( 0, 0, 0, 0 );
	CImage img;
	img.Load( initPng(MAKEINTRESOURCE(id)) );
	int32 width = img.GetWidth();
	int32 height = img.GetHeight();

	uint32 left=0,top=0,right=width,bottom=height;
	if ( pRect!=NULL )
	{
		left = pRect->left;
		top = pRect->top;
		right = getMin( pRect->right, width );
		bottom = getMin( pRect->bottom, height );
	}

	unsigned char* pCol = 0;
	HRGN rgnTemp;
	for ( uint32 y=top; y<bottom; y++ )
	{
		uint8* pPix = NULL;
		uint32 x = 0;
		do  
		{
			while( x<right )
			{
				pCol = (unsigned char*)img.GetPixelAddress(x,y);
				unsigned char alpha = pCol[3];
				if ( alpha != 255 )
					x++;
				else
					break;
			}
			if ( x >= right )
				break;
			int32 iLeftX = x;
			while( x<right && img.GetPixel(x,y)!=255 )
			{

				pCol = (unsigned char*)img.GetPixelAddress(x,y);
				unsigned char alpha = pCol[3];
				if ( alpha == 255 )
					++x;
				else
					break;
			}
			rgnTemp = CreateRectRgn( iLeftX, y, x, y+1 );
			CombineRgn( hRgn, hRgn, rgnTemp, RGN_OR );
			DeleteObject( rgnTemp );
		}
		while( x < right );
	}
	return hRgn;
}


Gdiplus::Image* imageFromResource( HINSTANCE hInst, uint32 id, char* str )
{
	HRSRC hRsrc = ::FindResource( hInst,MAKEINTRESOURCE(id), str );
	if ( !hRsrc )
		return NULL;
	DWORD len = SizeofResource( hInst, hRsrc );
	BYTE* lpRsrc = (BYTE*)LoadResource( hInst, hRsrc );
	if ( !lpRsrc )
		return NULL;  
	HGLOBAL hMem = GlobalAlloc( GMEM_FIXED, len );  
	BYTE* pmem = (BYTE*)GlobalLock( hMem );
	memcpy( pmem, lpRsrc, len );
	IStream* pstm;
	CreateStreamOnHGlobal( hMem,FALSE, &pstm );  
	Gdiplus::Image* img = Gdiplus::Image::FromStream( pstm );
	GlobalUnlock( hMem );
	pstm->Release();  
	FreeResource( lpRsrc );
	return img;  
} 
