#pragma once

class CListCtrl;
class CMyList
{
public:
	CMyList() {};
	~CMyList() {};
	void set_list_ctrl(CListCtrl * pListCtrl) { m_pListCtrl = pListCtrl; };
	void init();
	void set_img(std::string strSetName ,const _stImgInfo stImg,int nIndex = -1);
public:

protected:
	CListCtrl * m_pListCtrl;
};
