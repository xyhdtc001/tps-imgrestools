
// imageResToolsDlg.cpp : 实现文件
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


// CimageResToolsDlg 对话框

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


// CimageResToolsDlg 消息处理程序

void CimageResToolsDlg::initDlgItemText()
{
	std::ifstream iFile("temptext");
	if (iFile)
	{
		while (1)
		{
			string  line;
			//IDC_EDIT_EXEPATH 工具目录.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_EXEPATH)->SetWindowText(line.c_str());

			//IDC_EDIT_PACKPATH 图片资源目录.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_PACKPATH)->SetWindowText(line.c_str());

			//IDC_EDIT_OUTPUT 输出目录.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_OUTPUT)->SetWindowText(line.c_str());

			//IDC_EDIT_DIR_LAN 海外资源目录.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_DIR_LAN)->SetWindowText(line.c_str());

			//IDC_EDIT_RUNPATHDIR runpath目录 .
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_RUNPATHDIR)->SetWindowText(line.c_str());

			//IDC_EDIT_EXEPATH 工具目录.
			if (!getline(iFile, line))
			{
				break;
			}
			GetDlgItem(IDC_EDIT_EXEPATH)->SetWindowText(line.c_str());

			//IDC_EDIT_EX_RESXML imgxml目录.
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
		//IDC_EDIT_EXEPATH 工具目录.
		GetDlgItem(IDC_EDIT_EXEPATH)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_PACKPATH 图片资源目录.

		GetDlgItem(IDC_EDIT_PACKPATH)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_OUTPUT 输出目录.

		GetDlgItem(IDC_EDIT_OUTPUT)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_DIR_LAN 海外资源目录.

		GetDlgItem(IDC_EDIT_DIR_LAN)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_RUNPATHDIR runpath目录 .

		GetDlgItem(IDC_EDIT_RUNPATHDIR)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_EXEPATH 工具目录.
		GetDlgItem(IDC_EDIT_EXEPATH)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
		//IDC_EDIT_EX_RESXML imgxml目录.
		GetDlgItem(IDC_EDIT_EX_RESXML)->GetWindowText(strText);
		oFile << strText.GetBuffer() << endl;
	}
}

BOOL CimageResToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	initDlgItemText();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CimageResToolsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CimageResToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CimageResToolsDlg::OnBnClickedButton1()
{
	//开始打包.
	CString strExePath;
	GetDlgItemText(IDC_EDIT_EXEPATH, strExePath);

	CString strPackPath;
	GetDlgItemText(IDC_EDIT_PACKPATH, strPackPath);

	CButton *pBtn = (CButton*)(GetDlgItem(IDC_CHECK_CHILDDIR));
	bool bChildDir = pBtn->GetCheck() == TRUE;


	// 设置参数.
	CImgPack imgPack;
	imgPack;



}


void CimageResToolsDlg::OnBnClickedButton3()
{
	// 同步.
}


void CimageResToolsDlg::OnClose()
{
	saveDlgItemText();
	CDialogEx::OnClose();
}
