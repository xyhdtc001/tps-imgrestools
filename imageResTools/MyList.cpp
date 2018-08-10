#include "stdafx.h"
#include "MyList.h"


void CMyList::init()
{
	if (!m_pListCtrl)
		return;

	LONG lStyle;
	lStyle = GetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE);//获取当前窗口的风格
	lStyle &= ~LVS_TYPEMASK;                                            //清除显示方式位
	lStyle = lStyle | LVS_REPORT | LVS_SHOWSELALWAYS;//设置报表风格
	SetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE, lStyle);//设置窗口风格



																 //选中某行时使整行高亮（只适用于报表风格的ListCtrl）
	DWORD dwStyle = m_pListCtrl->GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用于报表风格的ListCtrl）


	m_pListCtrl->DeleteAllItems();
	m_pListCtrl->SetExtendedStyle(dwStyle);          //设置扩展风格
	m_pListCtrl->SetBkColor(RGB(0xC7, 0xED, 0xCC));     //设置背景颜色
	m_pListCtrl->SetTextBkColor(RGB(0xC7, 0xED, 0xCC)); //设置文本背景颜色
	m_pListCtrl->SetTextColor(RGB(0, 0, 0));      //设置文本颜色
	m_pListCtrl->InsertColumn(0, "图片集", LVCFMT_LEFT, 120);
	m_pListCtrl->InsertColumn(1, "名称", LVCFMT_LEFT, 230);
	m_pListCtrl->InsertColumn(2, "位置X", LVCFMT_LEFT, 60);
	m_pListCtrl->InsertColumn(3, "位置Y", LVCFMT_LEFT, 60);
	m_pListCtrl->InsertColumn(4, "宽度", LVCFMT_LEFT, 60);
	m_pListCtrl->InsertColumn(5, "高度", LVCFMT_LEFT, 60);

}

void CMyList::set_img(std::string strSetName,const _stImgInfo stImg, int nIndex /*= -1*/)
{
	if (nIndex == -1)
	{
		nIndex = m_pListCtrl->GetItemCount();
		//nIndex = nIndex + 1;
	}
	m_pListCtrl->InsertItem(nIndex, strSetName.c_str());//插入一条
	m_pListCtrl->SetItemText(nIndex, 1, stImg.strName.c_str());//开始写内容
	char sz[100];
	sprintf(sz, "%d", stImg.nPosX);
	m_pListCtrl->SetItemText(nIndex, 2, sz);
	sprintf(sz, "%d", stImg.nPosY);
	m_pListCtrl->SetItemText(nIndex, 3, sz);
	sprintf(sz, "%d", stImg.nWidth);
	m_pListCtrl->SetItemText(nIndex, 4, sz);
	sprintf(sz, "%d", stImg.nHeigth);
	m_pListCtrl->SetItemText(nIndex, 5, sz);
}
