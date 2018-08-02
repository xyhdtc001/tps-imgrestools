#pragma once

#include "windows.h"
#include <vector>
#include <map>
#include "basecode/BaseType.h"
#include "basecode/StringData.h"
#include "basecode/ui_func.h"

#define WM_CONTENT_CMD WM_USER + 100

using namespace std;

class ImgManageLayer
{
public:
	ImgManageLayer();
	~ImgManageLayer();
	void ImgManageLayer::init( HINSTANCE hInst );
	Gdiplus::Image* getImg( uint32 id );
	Gdiplus::Image* getImgJpg( uint32 id );

protected:
	map<uint32, Gdiplus::Image* > _mapImg;
	HINSTANCE _hInst;
	
};

class ImgContentLayer
{
public:
	ImgContentLayer(void);
	~ImgContentLayer(void);
	bool init( HWND hWnd, HINSTANCE hInst );
	void setMoveTitleHeight( uint32 height );

	void paint();
	void resize( uint32 width, uint32 height );
	void addImgItem( const RECT& rect, uint32 resource_id, uint32 cmd=0 );
	void addImgItem( const RECT& rect, const RECT& srcRect, uint32 resource_id, uint32 cmd=0 );

	void addImgItem( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd=0 );
	void ImgContentLayer::addImgItem( const RECT& rect, uint32 normal, RECT& normalRect,
							uint32 over, RECT& overRect, uint32 click, RECT& clickRect, uint32 cmd );

	void addImgItemJpg( const RECT& rect, uint32 resource_id, uint32 cmd=0 );
	void addImgItemJpg( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd=0 );
	void addTxtItem( const RECT& rect, const Data& str, uint32 cmd=0 );
	bool WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

	void Update();
	//void UpdateRect( RECT& rect );

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
		Gdiplus::Image* pImg;
	};
	struct HookItem
	{
		ImgContentLayer* pContent;
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
		Gdiplus::Image* pImgNormal;
		RECT rectNormal;
		Gdiplus::Image* pImgOver;
		RECT rectOver;
		Gdiplus::Image* pImgClick;
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
	static ImgManageLayer _imgManage;
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
