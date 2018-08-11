
// imageResToolsDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "MyList.h"

// CimageResToolsDlg 对话框
class CimageResToolsDlg : public CDialogEx
{
// 构造
public:
	CimageResToolsDlg(CWnd* pParent = NULL);	// 标准构造函数

	~CimageResToolsDlg();

// 对话框数据
	enum { IDD = IDD_IMAGERESTOOLS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;


	void initDlgItemText();

	void saveDlgItemText();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void update_list(CMyList* pList, const std::map<string, _stImgSetInfo> & mapInfo,int nPos);
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnClose();
	CListCtrl m_listOutPutImgInfo;
	CListCtrl m_listAddInfo;
	CListCtrl m_listDeeInfo;

	CMyList m_mylistOutPutImgInfo;
	CMyList m_mylistAddInfo;
	CMyList m_mylistDeeInfo;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();

	std::map<string, _stImgSetInfo> m_curMapDee;
};
