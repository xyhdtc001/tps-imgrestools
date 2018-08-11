
// imageResToolsDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "MyList.h"

// CimageResToolsDlg �Ի���
class CimageResToolsDlg : public CDialogEx
{
// ����
public:
	CimageResToolsDlg(CWnd* pParent = NULL);	// ��׼���캯��

	~CimageResToolsDlg();

// �Ի�������
	enum { IDD = IDD_IMAGERESTOOLS_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;


	void initDlgItemText();

	void saveDlgItemText();

	// ���ɵ���Ϣӳ�亯��
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
