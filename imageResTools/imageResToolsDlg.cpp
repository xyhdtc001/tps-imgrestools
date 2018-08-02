
// imageResToolsDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "imageResTools.h"
#include "imageResToolsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#include <ostream>
#include <fstream>

#include "sync_handle.h"
#include "img_pack.h"


// CimageResToolsDlg �Ի���

using namespace std;

CimageResToolsDlg::CimageResToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CimageResToolsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CimageResToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CimageResToolsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CimageResToolsDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CimageResToolsDlg::OnBnClickedButton3)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CimageResToolsDlg ��Ϣ�������

void CimageResToolsDlg::initDlgItemText()
{
	std::ifstream iFile("temptext");
	if (iFile)
	{
		while (1)
		{
			string  line;
			//IDC_EDIT_EXEPATH ����Ŀ¼.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_EXEPATH)->SetWindowText(line.c_str());

			//IDC_EDIT_PACKPATH ͼƬ��ԴĿ¼.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_PACKPATH)->SetWindowText(line.c_str());

			//IDC_EDIT_OUTPUT ���Ŀ¼.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(line.c_str());

			//IDC_EDIT_DIR_LAN ������ԴĿ¼.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_DIR_LAN)->SetWindowText(line.c_str());

			//IDC_EDIT_RUNPATHDIR runpathĿ¼ .
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_RUNPATHDIR)->SetWindowText(line.c_str());

			//IDC_EDIT_EXEPATH ����Ŀ¼.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_EXEPATH)->SetWindowText(line.c_str());

			//IDC_EDIT_EX_RESXML imgxmlĿ¼.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_EX_RESXML)->SetWindowText(line.c_str());
		}
	}
}

void CimageResToolsDlg::saveDlgItemText()
{
	std::ofstream oFile("temptext");
	if (oFile)
	{
		CString strText;

		string  line;
		//IDC_EDIT_EXEPATH ����Ŀ¼.
		GetDlgItem(IDC_EDIT_EXEPATH)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_PACKPATH ͼƬ��ԴĿ¼.

		GetDlgItem(IDC_EDIT_PACKPATH)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_OUTPUT ���Ŀ¼.

		GetDlgItem(IDC_EDIT_OUTPUT)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_DIR_LAN ������ԴĿ¼.

		GetDlgItem(IDC_EDIT_DIR_LAN)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_RUNPATHDIR runpathĿ¼ .

		GetDlgItem(IDC_EDIT_RUNPATHDIR)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_EXEPATH ����Ŀ¼.
		GetDlgItem(IDC_EDIT_EXEPATH)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_EX_RESXML imgxmlĿ¼.
		GetDlgItem(IDC_EDIT_EX_RESXML)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
	}
}

BOOL CimageResToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	initDlgItemText();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CimageResToolsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CimageResToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CimageResToolsDlg::OnBnClickedButton1()
{
	//��ʼ���.
	CString strExePath;
	GetDlgItemText(IDC_EDIT_EXEPATH, strExePath);

	CString strPackPath;
	GetDlgItemText(IDC_EDIT_PACKPATH, strPackPath);

	CButton *pBtn = (CButton*)(GetDlgItem(IDC_CHECK_CHILDDIR));
	bool bChildDir = pBtn->GetCheck() == TRUE;


	// ���ò���.
	CImgPack imgPack;
	imgPack;



}


void CimageResToolsDlg::OnBnClickedButton3()
{
	// ͬ��.
}


void CimageResToolsDlg::OnClose()
{
	saveDlgItemText();
	CDialogEx::OnClose();
}
