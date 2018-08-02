#include "ImgContent.h"
#include "basecode/ui_func.h"
#include "commctrl.h"

map<HWND,ImgContent::HookItem> ImgContent::_mapContent;
ImgManage ImgContent::_imgManage;

ImgContent::ImgContent(void)
{
	_bTrackMouse = FALSE;
	_bChangeTrackState = FALSE;
	_blackBrush = CreateSolidBrush( RGB(0,0,0) );
	_iMoveTitleHeight = 0;
	_bMoveWindow = false;
}


ImgContent::~ImgContent(void)
{
	DeleteObject( _blackBrush );
}


LONG ImgContentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	map<HWND,ImgContent::HookItem>::iterator iter = ImgContent::_mapContent.find( hwnd );
	if ( iter == ImgContent::_mapContent.end() )
		return NULL;
	if ( iter->second.pContent->WndProc(hwnd,msg,wParam,lParam) )
		return iter->second.wndProc( hwnd, msg, wParam, lParam );
	else
		return S_OK;
}


bool ImgContent::WndProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
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
						iter->pImgOver->AlphaBlend( _hBackDC, iter->rect, iter->rectOver, 0xff, AC_SRC_OVER );
						InvalidateRect( _hWnd, &iter->rect, TRUE );
					}
				}
				else
				{
					if ( iter->state == ITEM_OVER )
					{
						iter->state = ITEM_NORMAL;
						iter->pImgNormal->AlphaBlend( _hBackDC, iter->rect, iter->rectNormal, 0xff, AC_SRC_OVER );
						InvalidateRect( _hWnd, &iter->rect, TRUE );
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
						InvalidateRect( _hWnd, &iter->rect, TRUE );
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
						InvalidateRect( _hWnd, &iter->rect, TRUE );
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
					iter->pImgClick->AlphaBlend( _hBackDC, iter->rect, iter->rectClick, 0xff, AC_SRC_OVER );
					InvalidateRect( _hWnd, &iter->rect, TRUE );
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
						iter->pImgOver->AlphaBlend( _hBackDC, iter->rect, iter->rectOver, 0xff, AC_SRC_OVER );
						InvalidateRect( _hWnd, &iter->rect, TRUE );
					}
					else
					{
						iter->state = ITEM_NORMAL;
						iter->pImgNormal->AlphaBlend( _hBackDC, iter->rect, iter->rectNormal, 0xff, AC_SRC_OVER );
						InvalidateRect( _hWnd, &iter->rect, TRUE );
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
			paint( stru.hdc, stru.rcPaint );
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


bool ImgContent::init( HWND hWnd )
{
	HookItem item;
	item.pContent = this;
	item.wndProc = (WNDPROC)SetWindowLong( hWnd, GWL_WNDPROC, (LONG)ImgContentProc );
	_mapContent[hWnd] = item;

	_hWnd = hWnd;
	_hBackDC = ::CreateCompatibleDC( GetDC(hWnd) );
	SetBkMode( _hBackDC, TRANSPARENT );
	SetTextColor( _hBackDC, RGB(255,255,255) );

	HFONT hFont = CreateFont( 15,0,0,0,FW_NORMAL,false,false,false, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,DEFAULT_QUALITY, DEFAULT_PITCH,_T("Arial") );
	DeleteObject( SelectObject(_hBackDC,hFont) );
	
	return true;
}


void ImgContent::resize( uint32 width, uint32 height )
{
	_hBackBitmap = CreateCompatibleBitmap( GetDC(_hWnd), width, height );
	DeleteObject( SelectObject(_hBackDC,_hBackBitmap) );

}


void ImgContent::addImgItem( const RECT& rect, uint32 resource_id, uint32 cmd )
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
	repaint();
}

void ImgContent::addImgItem( const RECT& rect, const RECT& srcRect, uint32 resource_id, uint32 cmd )
{
	ImgItem item;
	item.cmd = cmd;
	item.pImg = _imgManage.getImg( resource_id );
	item.rect = rect;
	item.srcRect = srcRect;
	_vecImgItem.push_back( item );
	repaint();
}


void ImgContent::addImgItemJpg( const RECT& rect, uint32 resource_id, uint32 cmd )
{
	ImgItem item;
	item.cmd = cmd;
	item.pImg = _imgManage.getImgJpg( resource_id );
	item.rect = rect;
	_vecImgItem.push_back( item );
	repaint();
}


void ImgContent::addImgItemJpg( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd )
{
	Img3Item item;
	item.rect = rect;
	item.cmd = cmd;

	item.pImgNormal = _imgManage.getImgJpg( normal );
	item.pImgOver = _imgManage.getImgJpg( over );
	item.pImgClick = _imgManage.getImgJpg( click );
	_vecImg3Item.push_back( item );
	repaint();
}


void ImgContent::addImgItem( const RECT& rect, uint32 normal, uint32 over, uint32 click, uint32 cmd )
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
	repaint();
}

void ImgContent::addImgItem( const RECT& rect, uint32 normal, RECT& normalRect,
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
	repaint();
}


void ImgContent::repaint()
{
	RECT rect;
	GetClientRect( _hWnd, &rect );
	::FillRect( _hBackDC, &rect, _blackBrush );

	for ( std::vector<ImgItem>::iterator iter=_vecImgItem.begin(); iter!=_vecImgItem.end(); iter++ )
	{
		RECT srcRect = {0,0,iter->rect.right-iter->rect.left,iter->rect.bottom-iter->rect.top};
		iter->pImg->AlphaBlend( _hBackDC, iter->rect, srcRect, 0xff, AC_SRC_OVER );
	}
	for ( std::vector<Img3Item>::iterator iter=_vecImg3Item.begin(); iter!=_vecImg3Item.end(); iter++ )
	{
		switch ( iter->state )
		{
		case ITEM_NORMAL:
			iter->pImgNormal->AlphaBlend( _hBackDC, iter->rect, iter->rectNormal, 0xff, AC_SRC_OVER );
			break;
		case ITEM_OVER:
			iter->pImgOver->AlphaBlend( _hBackDC, iter->rect, iter->rectOver, 0xff, AC_SRC_OVER );
			break;
		case ITEM_CLICK:
			iter->pImgClick->AlphaBlend( _hBackDC, iter->rect, iter->rectClick, 0xff, AC_SRC_OVER );
			break;
		}
	}
	for ( std::vector<TxtItem>::iterator iter=_vecTxtItem.begin(); iter!=_vecTxtItem.end(); iter++ )
	{
		::SetTextColor( _hBackDC, RGB(255,255,255) );
		::DrawText( _hBackDC, iter->txt.c_str(), iter->txt.length(), &iter->rect, DT_LEFT );
	}
}


void ImgContent::paint( HDC hDC, const RECT& rect )
{
	BitBlt( hDC, rect.left, rect.top, rect.right-rect.left,
				rect.bottom-rect.top, _hBackDC, rect.left, rect.top, SRCCOPY );
}


void ImgContent::addTxtItem( const RECT& rect, const Data& str, uint32 cmd/*=0 */ )
{
	TxtItem item;
	item.cmd = cmd;
	item.rect = rect;
	item.txt = str;
	_vecTxtItem.push_back( item );
}

void ImgContent::setMoveTitleHeight( uint32 height )
{
	_iMoveTitleHeight = height;
}

ImgManage::ImgManage()
{

}


ImgManage::~ImgManage()
{
	for ( map<uint32,CImage*>::iterator iter=_mapImg.begin(); iter!=_mapImg.end(); iter++ )
	{
		iter->second->Destroy();
		delete iter->second;
	}
	_mapImg.clear();
}


CImage* ImgManage::getImg( uint32 id )
{
	map<uint32,CImage*>::iterator iter = _mapImg.find( id );
	if ( iter == _mapImg.end() )
	{
		CImage* pImg = new CImage;
		pImg->Load( initPng(MAKEINTRESOURCE(id)) );
		parsePng( pImg );
		_mapImg[id] = pImg;
		return pImg;
	}
	return iter->second;
}


CImage* ImgManage::getImgJpg( uint32 id )
{
	map<uint32,CImage*>::iterator iter = _mapImg.find( id );
	if ( iter == _mapImg.end() )
	{
		CImage* pImg = new CImage;
		pImg->Load( initJpg(MAKEINTRESOURCE(id)) );
		parsePng( pImg );
		_mapImg[id] = pImg;
		return pImg;
	}
	return iter->second;
}
