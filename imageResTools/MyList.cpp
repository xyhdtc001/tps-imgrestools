#include "stdafx.h"
#include "MyList.h"


void CMyList::init()
{
	if (!m_pListCtrl)
		return;

	LONG lStyle;
	lStyle = GetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE);//��ȡ��ǰ���ڵķ��
	lStyle &= ~LVS_TYPEMASK;                                            //�����ʾ��ʽλ
	lStyle = lStyle | LVS_REPORT | LVS_SHOWSELALWAYS;//���ñ�����
	SetWindowLong(m_pListCtrl->m_hWnd, GWL_STYLE, lStyle);//���ô��ڷ��



																 //ѡ��ĳ��ʱʹ���и�����ֻ�����ڱ������ListCtrl��
	DWORD dwStyle = m_pListCtrl->GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;
	dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ�����ڱ������ListCtrl��


	m_pListCtrl->DeleteAllItems();
	m_pListCtrl->SetExtendedStyle(dwStyle);          //������չ���
	m_pListCtrl->SetBkColor(RGB(0xC7, 0xED, 0xCC));     //���ñ�����ɫ
	m_pListCtrl->SetTextBkColor(RGB(0xC7, 0xED, 0xCC)); //�����ı�������ɫ
	m_pListCtrl->SetTextColor(RGB(0, 0, 0));      //�����ı���ɫ
	m_pListCtrl->InsertColumn(0, "ͼƬ��", LVCFMT_LEFT, 120);
	m_pListCtrl->InsertColumn(1, "����", LVCFMT_LEFT, 230);
	m_pListCtrl->InsertColumn(2, "λ��X", LVCFMT_LEFT, 60);
	m_pListCtrl->InsertColumn(3, "λ��Y", LVCFMT_LEFT, 60);
	m_pListCtrl->InsertColumn(4, "���", LVCFMT_LEFT, 60);
	m_pListCtrl->InsertColumn(5, "�߶�", LVCFMT_LEFT, 60);

}

void CMyList::set_img(std::string strSetName,const _stImgInfo stImg, int nIndex /*= -1*/)
{
	if (nIndex == -1)
	{
		nIndex = m_pListCtrl->GetItemCount();
		//nIndex = nIndex + 1;
	}
	m_pListCtrl->InsertItem(nIndex, strSetName.c_str());//����һ��
	m_pListCtrl->SetItemText(nIndex, 1, stImg.strName.c_str());//��ʼд����
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
