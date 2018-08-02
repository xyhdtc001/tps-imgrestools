#include "ImgContentLayer.h"
#include "basecode/ui_func.h"
#include "commctrl.h"

map<HWND,ImgContentLayer::HookItem> ImgContentLayer::_mapContent;
ImgManageLayer ImgContentLayer::_imgManage;

using namespace Gdiplus;

ImgContentLayer::ImgContentLayer(void)
{
	_bTrackMouse = FALSE;
	_bChangeTrackState = FALSE;
	_blackBrush = CreateSolidBrush( RGB(0,0,0) );
	_iMoveTitleHeight = 0;
	_bMoveWindow = false;
}


ImgContentLayer::~ImgContentLayer(void)
{
	DeleteObject( _blackBrush );
}


LONG ImgContentLayerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	map<HWND,ImgContentLayer::HookItem>::iterator iter = ImgContentLayer::_mapContent.find( hwnd );
	if ( iter == ImgContentLayer::_mapContent.end() )
		return NULL;
	if ( iter->second.pContent->WndProc(hwnd,msg,wParam,lParam) )
		return iter->second.wndProc( hwnd, msg, wParam, lParam );
	else
		return S_OK;
}


bool ImgContentLayer::WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	Gdiplus::Graphics graph( _hBackDC );
	//graph.SetSmoothingMode( SmoothingModeAntiAlias );
	switch ( msg )
	{
	case WM_MOUSELEAVE:
		_bChangeTrackState = TRUE;
		lParam = 0;
	case WM_MOUSEMOVE:
		{
			if ( _iMoveTitleHeight > 0 )
			{
				if ( _bMoveWindow )
				{
					POINT pt;
					GetCursorPos( &pt );
					RECT rect;
					GetWindowRect( _hWnd, &rect );
					int left = rect.left + pt.x - _ptDragMouse.x;
					int top = rect.top + pt.y - _ptDragMouse.y;
					MoveWindow( _hWnd, left, top, rect.right-rect.left, rect.bottom-rect.top, TRUE );
					_ptDragMouse = pt;
				}
			}
			if ( !_bTrackMouse )
			{
				TRACKMOUSEEVENT tme;  
				tme.cbSize = sizeof(tme);  
				tme.hwndTrack = hwnd;  
				tme.dwFlags = TME_LEAVE|TME_HOVER;  
				tme.dwHoverTime = 1;  
				_bTrackMouse = _TrackMouseEvent( &tme );
			}
			int32 x = LOWORD(lParam);
			int32 y = HIWORD(lParam);
			if ( _bChangeTrackState )
			{
				_bTrackMouse = FALSE;
				_bChangeTrackState = FALSE;
				x = -999;
				y = -999;
			}
			for ( std::vector<Img3Item>::iterator iter=_vecImg3Item.begin(); iter!=_vecImg3Item.end(); iter++ )
			{
				POINT pt = {x,y};
				if ( PtInRect(&iter->rect,pt) )
				{
					if ( iter->state == ITEM_NORMAL )
					{
						iter->state = ITEM_OVER;
						graph.DrawImage( iter->pImgOver, iter->rect.left, iter->rect.top, iter->rectOver.left, iter->rectOver.top,
										iter->rectOver.right-iter->rectOver.left, iter->rectOver.bottom-iter->rectOver.top, UnitPixel );
						Update();
					}
				}
				else
				{
					if ( iter->state == ITEM_OVER )
					{
						iter->state = ITEM_NORMAL;
						graph.DrawImage( iter->pImgNormal, iter->rect.left, iter->rect.top, iter->rectNormal.left, iter->rectNormal.top,
									iter->rectNormal.right-iter->rectNormal.left, iter->rectNormal.bottom-iter->rectNormal.top, UnitPixel );
						Update();
					}
				}
			}

			for ( std::vector<TxtItem>::iterator iter=_vecTxtItem.begin(); iter!=_vecTxtItem.end(); iter++ )
			{
				POINT pt = {x,y};
				if ( PtInRect(&iter->rect,pt) )
				{
					if ( iter->state == ITEM_NORMAL )
					{
						iter->state = ITEM_OVER;
						RECT srcRect = {0,0,iter->rect.right-iter->rect.left,iter->rect.bottom-iter->rect.top};
						::SetTextColor( _hBackDC, RGB(255,0,0) );
						::DrawText( _hBackDC, iter->txt.c_str(), iter->txt.length(), &iter->rect, DT_LEFT );				
					}
				}
				else
				{
					if ( iter->state == ITEM_OVER )
					{
						iter->state = ITEM_NORMAL;
						RECT srcRect = {0,0,iter->rect.right-iter->rect.left,iter->rect.bottom-iter->rect.top};
						::SetTextColor( _hBackDC, RGB(255,255,255) );
						::DrawText( _hBackDC, iter->txt.c_str(), iter->txt.length(), &iter->rect, DT_LEFT );				
					}
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:
		{
			if ( _iMoveTitleHeight > 0 )
			{
				uint32 y = HIWORD( lParam );
				if ( y < _iMoveTitleHeight )
				{
					_bMoveWindow = true;
					GetCursorPos( &_ptDragMouse );
					SetCapture( _hWnd );
				}
			}

			int32 x = LOWORD(lParam);
			int32 y = HIWORD(lParam);
			POINT pt = {x,y};
			for ( std::vector<Img3Item>::iterator iter=_vecImg3Item.begin(); iter!=_vecImg3Item.end(); iter++ )
			{
				if ( PtInRect(&iter->rect,pt) )
				{
					iter->state = ITEM_CLICK;
					graph.DrawImage( iter->pImgClick, iter->rect.left, iter->rect.top, iter->rectClick.left, iter->rectClick.top,
								iter->rectClick.right-iter->rectClick.left, iter->rectClick.bottom-iter->rectClick.top, UnitPixel );
					Update();
				}
			}
			for ( std::vector<TxtItem>::iterator iter=_vecTxtItem.begin(); iter!=_vecTxtItem.end(); iter++ )
			{
				if ( PtInRect(&iter->rect,pt) )
				{
					iter->state = ITEM_CLICK;
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			if ( _bMoveWindow )
				ReleaseCapture();
			_bMoveWindow = false;
			int32 x = LOWORD(lParam);
			int32 y = HIWORD(lParam);
			POINT pt = {x,y};
			for ( std::vector<Img3Item>::iterator iter=_vecImg3Item.begin(); iter!=_vecImg3Item.end(); iter++ )
			{
				if ( iter->state == ITEM_CLICK )
				{
					if ( PtInRect(&iter->rect,pt) )
					{
						iter->state = ITEM_OVER;
						graph.DrawImage( iter->pImgOver, iter->rect.left, iter->rect.top, iter->rectOver.left, iter->rectOver.top,
									iter->rectOver.right-iter->rectOver.left, iter->rectOver.bottom-iter->rectOver.top, UnitPixel );
						Update();
					}
					else
					{
						iter->state = ITEM_NORMAL;
						graph.DrawImage( iter->pImgNormal, iter->rect.left, iter->rect.top, iter->rectNormal.left, iter->rectNormal.top,
									iter->rectNormal.right-iter->rectNormal.left, iter->rectNormal.bottom-iter->rectNormal.top, UnitPixel );
						Update();
					}
					if ( iter->cmd != 0 )
						PostMessage( _hWnd, WM_CONTENT_CMD, iter->cmd, 0 );
				}
			}
			for ( std::vector<TxtItem>::iterator iter=_vecTxtItem.begin(); iter!=_vecTxtItem.end(); iter++ )
			{
				if ( iter->state == ITEM_CLICK )
				{
					if ( PtInRect(&iter->rect,pt) )
					{
						iter->state = ITEM_OVER;
					}
					else
					{
						iter->state = ITEM_NORMAL;
					}
					if ( iter->cmd != 0 )
						PostMessage( _hWnd, WM_CONTENT_CMD, iter->cmd, 0 );
				}
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT stru;
			BeginPaint( hwnd, &stru );
			EndPaint( hwnd, &stru );
			return false;
		}
		break;
	case WM_ERASEBKGND:
		return false;
		break;
	case WM_SIZE:
		resize( LOWORD(lParam), HIWORD(lParam) );
		return false;
		break;
	default:
		break;
	}
	return true;
}


bool ImgContentLayer::init( HWND hWnd, HINSTANCE hInst )
{
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;  
	ULONG_PTR m_gdiplusToken;   
	GdiplusStartup( &m_gdiplusToken, &m_gdiplusStartupInput, NULL ); 

	DWORD dwExStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
	if ((dwExStyle & 0x80000) != 0x80000)
	{
		::SetWindowLong(hWnd, GWL_EXSTYLE, dwExStyle^0x80000);
	}

	_imgManage.init( hInst );
	HookItem item;
	item.pContent = this;
	item.wndProc = (WNDPROC)SetWindowLong( hWnd, GWL_WNDPROC, (LONG)ImgContentLayerProc );
	_mapContent[hWnd] = item;

	_hWnd = hWnd;
	_hBackDC = ::CreateCompatibleDC( GetWindowDC(hWnd) );

	SetBkMode( _hBackDC, TRANSPARENT );
	SetTextColor( _hBackDC, RGB(255,255,255) );
	
	HFONT hFont = CreateFont( 15,0,0,0,FW_NORMAL,false,false,false, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,DEFAULT_QUALITY, DEFAULT_PITCH,_T("Arial") );
	DeleteObject( SelectObject(_hBackDC,hFont) );

	return true;
}


void ImgContentLayer::resize( uint32 width, uint32 height )
{	
	_hBackBitmap = CreateCompatibleBitmap( GetWindowDC(_hWnd), width, height );
	DeleteObject( SelectObject(_hBackDC,_hBackBitmap) );
	paint();
}


void ImgContentLayer::addImgItem( const RECT& rect, uint32 resource_id, uint32 cmd )
{
	ImgItem item;
	item.cmd = cmd;
	item.pImg = _imgManage.getImg( resource_id );
	item.rect = rect;
	item.rect.left = 0;
	item.rect.top = 0;
	item.rect.right = rect.right-rect.left;
	item.rect.bottom = rect.bottom-rect.top;
	_vecImgItem.push_back( item );
	paint();
}

void ImgContentLayer::addImgItem( const RECT& rect, const RECT& srcRect, uint32 resource_id, uint32 cmd )
{
	ImgItem item;
	item.cmd = cmd;
	item.pImg = _imgManage.getImg( resource_id );
	item.rect = rect;
	item.srcRect = srcRect;
	_vecImgItem.push_back( item );
	paint();
}


void ImgContentLayer::addImgItemJpg( const RECT& rect, uint32 resource_id, uint32 cmd )
{
	ImgItem item;
	item.cmd = cmd;
	item.pImg = _imgManage.getImgJpg( resource_id );
	item.rect = rect;
	_vecImgItem.push_back( item );
	paint();
}


void ImgContentLayer::addImgItemJpg( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd )
{
	Img3Item item;
	item.rect = rect;
	item.cmd = cmd;

	item.pImgNormal = _imgManage.getImgJpg( normal );
	item.pImgOver = _imgManage.getImgJpg( over );
	item.pImgClick = _imgManage.getImgJpg( click );
	_vecImg3Item.push_back( item );
	paint();
}


void ImgContentLayer::addImgItem( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd )
{
	Img3Item item;
	item.rect = rect;
	item.cmd = cmd;

	item.pImgNormal = _imgManage.getImg( normal );
	item.rectNormal.left = 0;
	item.rectNormal.top = 0;
	item.rectNormal.right = item.pImgNormal->GetWidth();
	item.rectNormal.bottom = item.pImgNormal->GetHeight();

	item.pImgOver = _imgManage.getImg( over );
	item.rectOver.left = 0;
	item.rectOver.top = 0;
	item.rectOver.right = item.pImgNormal->GetWidth();
	item.rectOver.bottom = item.pImgNormal->GetHeight();

	item.pImgClick = _imgManage.getImg( click );
	item.rectClick.left = 0;
	item.rectClick.top = 0;
	item.rectClick.right = item.pImgNormal->GetWidth();
	item.rectClick.bottom = item.pImgNormal->GetHeight();

	_vecImg3Item.push_back( item );
	paint();
}

void ImgContentLayer::addImgItem( const RECT& rect, uint32 normal, RECT& normalRect,
						uint32 over, RECT& overRect, uint32 click, RECT& clickRect, uint32 cmd )
{
	Img3Item item;
	item.rect = rect;
	item.cmd = cmd;

	item.pImgNormal = _imgManage.getImg( normal );
	item.rectNormal = normalRect;
	
	item.pImgOver = _imgManage.getImg( over );
	item.rectOver = overRect;

	item.pImgClick = _imgManage.getImg( click );
	item.rectClick = clickRect;

	_vecImg3Item.push_back( item );
	paint();
}


void ImgContentLayer::paint()
{
	Gdiplus::Graphics graph( _hBackDC );
	/*
	graph.SetSmoothingMode( SmoothingModeAntiAlias );
	graph.SetInterpolationMode( InterpolationModeNearestNeighbor  );
	graph.SetCompositingQuality( CompositingQualityHighQuality );
	graph.SetCompositingMode( CompositingModeSourceOver );
	graph.SetPixelOffsetMode( PixelOffsetModeHalf  );
	*/
	for ( std::vector<ImgItem>::iterator iter=_vecImgItem.begin(); iter!=_vecImgItem.end(); iter++ )
	{
		graph.DrawImage( iter->pImg, iter->rect.left, iter->rect.top, 0, 0,
				iter->rect.right-iter->rect.left, iter->rect.bottom-iter->rect.top, UnitPixel );
	}
	for ( std::vector<Img3Item>::iterator iter=_vecImg3Item.begin(); iter!=_vecImg3Item.end(); iter++ )
	{
		switch ( iter->state )
		{
		case ITEM_NORMAL:
			graph.DrawImage( iter->pImgNormal, iter->rect.left, iter->rect.top, iter->rectNormal.left, iter->rectNormal.top,
							iter->rectNormal.right-iter->rectNormal.left, iter->rectNormal.bottom-iter->rectNormal.top, UnitPixel );
			break;
		case ITEM_OVER:
			graph.DrawImage( iter->pImgOver, iter->rect.left, iter->rect.top, iter->rectOver.left, iter->rectOver.top,
							iter->rectOver.right-iter->rectOver.left, iter->rectOver.bottom-iter->rectOver.top, UnitPixel );
			break;
		case ITEM_CLICK:
			graph.DrawImage( iter->pImgClick, iter->rect.left, iter->rect.top, iter->rectClick.left, iter->rectClick.top,
							iter->rectClick.right-iter->rectClick.left, iter->rectClick.bottom-iter->rectClick.top, UnitPixel );
			break;
		}
	}
	for ( std::vector<TxtItem>::iterator iter=_vecTxtItem.begin(); iter!=_vecTxtItem.end(); iter++ )
	{
		::SetTextColor( _hBackDC, RGB(255,255,255) );
		::DrawText( _hBackDC, iter->txt.c_str(), iter->txt.length(), &iter->rect, DT_LEFT );
	}
	Update();
}


void ImgContentLayer::Update()
{
	POINT ptSrc = {0,0};
	BLENDFUNCTION alBlend;
	alBlend.AlphaFormat = AC_SRC_ALPHA;
	alBlend.BlendOp = 0;
	alBlend.BlendFlags = 0;
	alBlend.SourceConstantAlpha = 0xFF;
	RECT wndRect;
	GetWindowRect( _hWnd, &wndRect );
	SIZE sizeWin = {wndRect.right-wndRect.left,wndRect.bottom-wndRect.top};
	POINT ptWnd = { wndRect.left,wndRect.top };
	::UpdateLayeredWindow( _hWnd, GetDC(_hWnd), &ptWnd, &sizeWin, _hBackDC, &ptSrc, 0, &alBlend, ULW_ALPHA );
}


/*
void ImgContentLayer::UpdateRect( RECT& rect )
{
	BLENDFUNCTION alBlend;
	alBlend.AlphaFormat = AC_SRC_ALPHA;
	alBlend.BlendOp = 0;
	alBlend.BlendFlags = 0;
	alBlend.SourceConstantAlpha = 0xFF;

	RECT wndRect;
	GetWindowRect( _hWnd, &wndRect );
	POINT ptDst = { wndRect.left, wndRect.top };
	POINT ptSrc = { 0, 0 };
	SIZE ptSize = { wndRect.right-wndRect.left, wndRect.bottom-wndRect.top };
	RECT dirtyRect = rect;

	UPDATELAYEREDWINDOWINFO info;
	info.cbSize = sizeof( UPDATELAYEREDWINDOWINFO );
	info.hdcDst = GetDC(  _hWnd );
	info.pptDst = &ptDst;
	info.psize = &ptSize;
	info.hdcSrc = _hBackDC;
	info.pptSrc = &ptSrc;
	info.crKey = RGB( 255,255,255 );
	info.pblend = &alBlend;
	info.prcDirty = &dirtyRect;
	info.dwFlags = ULW_ALPHA;
	::UpdateLayeredWindowIndirect( _hWnd, &info );
}
*/


void ImgContentLayer::addTxtItem( const RECT& rect, const Data& str, uint32 cmd/*=0 */ )
{
	TxtItem item;
	item.cmd = cmd;
	item.rect = rect;
	item.txt = str;
	_vecTxtItem.push_back( item );
}

void ImgContentLayer::setMoveTitleHeight( uint32 height )
{
	_iMoveTitleHeight = height;
}

ImgManageLayer::ImgManageLayer()
{

}


ImgManageLayer::~ImgManageLayer()
{
	for ( map<uint32,Image*>::iterator iter=_mapImg.begin(); iter!=_mapImg.end(); iter++ )
	{
		delete iter->second;
	}
	_mapImg.clear();
}


void ImgManageLayer::init( HINSTANCE hInst )
{
	_hInst = hInst;
}


Gdiplus::Image* ImgManageLayer::getImg( uint32 id )
{
	map<uint32,Image*>::iterator iter = _mapImg.find( id );
	if ( iter == _mapImg.end() )
	{
		Gdiplus::Image* pImg = imageFromResource( _hInst, id, "PNG" );
		_mapImg[id] = pImg;
		return pImg;
	}
	return iter->second;
}


Gdiplus::Image* ImgManageLayer::getImgJpg( uint32 id )
{
	map<uint32,Image*>::iterator iter = _mapImg.find( id );
	if ( iter == _mapImg.end() )
	{
		Gdiplus::Image* pImg = imageFromResource( _hInst, id, "JPG" );
		_mapImg[id] = pImg;
		return pImg;
	}
	return iter->second;
}
