
// imageResToolsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "imageResTools.h"
#include "imageResToolsDlg.h"
#include "afxdialogex.h"
#include "ImgResXmlOpreate.h"




#include <ostream>
#include <fstream>

#include "sync_handle.h"
#include "img_pack.h"
#include "img_tools_common.h"


// CimageResToolsDlg 对话框

using namespace std;

CimageResToolsDlg::CimageResToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CimageResToolsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CimageResToolsDlg::~CimageResToolsDlg()
{
	CImgToolComm::ReleaseInstance();
}

void CimageResToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_RESPACK, m_listOutPutImgInfo);
	DDX_Control(pDX, IDC_LIST_RES_ADD, m_listAddInfo);
	DDX_Control(pDX, IDC_LIST_RES_DEL, m_listDeeInfo);
}

BEGIN_MESSAGE_MAP(CimageResToolsDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CimageResToolsDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON3, &CimageResToolsDlg::OnBnClickedButton3)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON2, &CimageResToolsDlg::OnBnClickedButton2)
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

			//子目录打包.checkbutton
			if (!getline(iFile,line))
			{
				break;
			}
			if (line == "childpack=true")
			{
				((CButton*)GetDlgItem(IDC_CHECK_CHILDDIR))->SetCheck(1);
			}
			else
			{
				((CButton*)GetDlgItem(IDC_CHECK_CHILDDIR))->SetCheck(0);
			}
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
		//子目录打包.checkbutton
		if (((CButton*)GetDlgItem(IDC_CHECK_CHILDDIR)))
		{
			strText = "childpack=true";
			oFile << strText.GetBuffer() << endl;
		}
		else
		{
			strText = " ";
			oFile << strText.GetBuffer() << endl;
		}
		
	}
}

BOOL CimageResToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	((CButton*)GetDlgItem(IDC_CHECK_CHILDDIR))->SetCheck(1);
	// TODO:  在此添加额外的初始化代码
	initDlgItemText();

	m_mylistOutPutImgInfo.set_list_ctrl(&m_listOutPutImgInfo);
	m_mylistOutPutImgInfo.init();
	m_mylistAddInfo.set_list_ctrl(&m_listAddInfo);
	m_mylistAddInfo.init();
	m_mylistDeeInfo.set_list_ctrl(&m_listDeeInfo);
	m_mylistDeeInfo.init();


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



void CimageResToolsDlg::update_list(CMyList* pList, const std::map<string, _stImgSetInfo> & mapInfo,int nLinePos)
{
	if (!pList)
	{
		return;
	}
	for (std::map<string, _stImgSetInfo>::const_iterator itMap = mapInfo.begin(); itMap != mapInfo.end();++itMap)
	{
		for (IMGSETMAP::const_iterator itSetImg = itMap->second.imgMap.begin(); itSetImg != itMap->second.imgMap.end();++itSetImg)
		{
			pList->set_img(itMap->first, itSetImg->second);
		}	
	}
}

void CimageResToolsDlg::OnBnClickedButton1()
{
	//开始打包.
	CString strExePath;
	GetDlgItemText(IDC_EDIT_EXEPATH, strExePath);

	//tp exe 路径.
	CString strPackPath;
	GetDlgItemText(IDC_EDIT_PACKPATH, strPackPath);

	// 输出目录
	CString strOutPath;
	GetDlgItemText(IDC_EDIT_OUTPUT, strOutPath);

	//runpath 目录.
	CString strRunPathDir;
	GetDlgItemText(IDC_EDIT_RUNPATHDIR,strRunPathDir);


	//子目录.
	CButton *pBtn = (CButton*)(GetDlgItem(IDC_CHECK_CHILDDIR));
	bool bChildDir = pBtn->GetCheck() == TRUE;


	CImgToolComm::GetSignleInstance()->setWorkDir(strPackPath);


	//imgresxml 
	CImgResXmlOpreate imgResXmlLast; //上次的XML文件.(即将生成的XML中的新文件.)
	CImgResXmlOpreate imgResXmlCur;  //当前的XML文件.(目前runpath中的文件)

	imgResXmlCur.set_work_dir(strRunPathDir);
	imgResXmlLast.update_img_info();
	imgResXmlCur.set_lastxml_ptr(&imgResXmlLast);
	imgResXmlCur.set_tp_out_dir(strOutPath.GetBuffer());

	// 设置参数.
	CImgPack imgPack;
	imgPack.setMainDir(strPackPath.GetBuffer());

	imgPack.setOutPutDir(strOutPath.GetBuffer());

	imgPack.setTexturepackExePath(strExePath.GetBuffer());

	imgPack.setXmlPtr(&imgResXmlCur,&imgResXmlLast);

	bool bRes = imgPack.startPack(bChildDir);
	if(bRes)
	{
		//合并到XML里 .
		const std::vector<Data>& vecResDir = imgPack.getPackDirVec();
		for (int nIndex = 0; nIndex < vecResDir.size();++ nIndex)
		{
			std::map<string, _stImgSetInfo> curSetInfo;
			imgResXmlCur.joinXmlFile(vecResDir[nIndex].c_str(),&curSetInfo);
			update_list(&m_mylistOutPutImgInfo,curSetInfo,-1);
		}
	}

	imgResXmlCur.SaveFile("");
	//输出差异信息.
	update_list(&m_mylistAddInfo, imgResXmlCur.get_diff_add(), -1);
	update_list(&m_mylistDeeInfo, imgResXmlCur.get_diff_dee(), -1);


	if (bRes)
	{
		AfxMessageBox("sucessed!");
	}
	else
	{
		AfxMessageBox("error");
	}
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


void CimageResToolsDlg::OnBnClickedButton2()
{
	// 整理目录下XML
	//runpath 目录.
	CString strRunPathDir;
	GetDlgItemText(IDC_EDIT_RUNPATHDIR, strRunPathDir);

	CImgResXmlOpreate imgResXmlCur;  //当前的XML文件.(目前runpath中的文件)

	imgResXmlCur.set_work_dir(strRunPathDir);

	imgResXmlCur.SaveFile("");
}
