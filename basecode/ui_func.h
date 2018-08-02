#pragma once
#include "windows.h"
#include "atlimage.h"

IStream* initPng( LPTSTR res );

void parsePng( CImage* pImag );

IStream* initJpg( LPTSTR res );

HRGN getRghFromPng( uint32 id, RECT* pRect=NULL );

Gdiplus::Image* imageFromResource( HINSTANCE hInst, uint32 id, char* str );
