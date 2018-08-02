#pragma once

#include "windows.h"
#include <vector>
#include <map>
#include "basecode/BaseType.h"
#include "basecode/StringData.h"
#include "basecode/ui_func.h"

#define WM_CONTENT_CMD WM_USER + 100

using namespace std;

class ImgManage
{
public:
	ImgManage();
	~ImgManage();
	CImage* getImg( uint32 id );
	CImage* getImgJpg( uint32 id );
protected:
	map<uint32, CImage* > _mapImg;

};

class ImgContent
{
public:
	ImgContent(void);
	~ImgContent(void);
	bool init( HWND hWnd );
	void setMoveTitleHeight( uint32 height );

	void repaint();
	void paint( HDC hDC, const RECT& rect );
	void resize( uint32 width, uint32 height );
	void addImgItem( const RECT& rect, uint32 resource_id, uint32 cmd=0 );
	void addImgItem( const RECT& rect, const RECT& srcRect, uint32 resource_id, uint32 cmd=0 );

	void addImgItem( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd=0 );
	void ImgContent::addImgItem( const RECT& rect, uint32 normal, RECT& normalRect,
							uint32 over, RECT& overRect, uint32 click, RECT& clickRect, uint32 cmd );

	void addImgItemJpg( const RECT& rect, uint32 resource_id, uint32 cmd=0 );
	void addImgItemJpg( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd=0 );
	void addTxtItem( const RECT& rect, const Data& str, uint32 cmd=0 );
	bool WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );


public:

	enum ITEM_STATE
	{
		ITEM_NORMAL = 0,
		ITEM_OVER,
		ITEM_CLICK,
	};
	struct ImgItem
	{
		uint32 cmd;
		RECT rect;
		RECT srcRect;
		CImage* pImg;
	};
	struct HookItem
	{
		ImgContent* pContent;
		WNDPROC wndProc;
	};
	struct Img3Item
	{
		Img3Item()
		{
			state = ITEM_NORMAL;
		}
		uint32 cmd;
		ITEM_STATE state;
		RECT rect;	
		CImage* pImgNormal;
		RECT rectNormal;
		CImage* pImgOver;
		RECT rectOver;
		CImage* pImgClick;
		RECT rectClick;
	};
	struct TxtItem
	{
		TxtItem()
		{
			state = ITEM_NORMAL;
		}
		uint32 cmd;
		ITEM_STATE state;
		RECT rect;
		Data txt;
	};

public:
	static map<HWND,HookItem> _mapContent;
	static ImgManage _imgManage;
	std::vector<ImgItem> _vecImgItem;
	std::vector<Img3Item> _vecImg3Item;
	std::vector<TxtItem> _vecTxtItem;
	HDC			_hBackDC;
	HBITMAP		_hBackBitmap;
	HWND		_hWnd;
	HBRUSH		_blackBrush;
	BOOL		_bTrackMouse;
	BOOL		_bChangeTrackState;

	uint32		_iMoveTitleHeight;
	bool		_bMoveWindow;
	POINT		_ptDragMouse;
};
