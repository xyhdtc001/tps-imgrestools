
// imageResToolsDlg.cpp : ʵ���ļ�
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


// CimageResToolsDlg �Ի���

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

			//��Ŀ¼���.checkbutton
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
		//��Ŀ¼���.checkbutton
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	((CButton*)GetDlgItem(IDC_CHECK_CHILDDIR))->SetCheck(1);
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	initDlgItemText();

	m_mylistOutPutImgInfo.set_list_ctrl(&m_listOutPutImgInfo);
	m_mylistOutPutImgInfo.init();
	m_mylistAddInfo.set_list_ctrl(&m_listAddInfo);
	m_mylistAddInfo.init();
	m_mylistDeeInfo.set_list_ctrl(&m_listDeeInfo);
	m_mylistDeeInfo.init();


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
	//��ʼ���.
	CString strExePath;
	GetDlgItemText(IDC_EDIT_EXEPATH, strExePath);

	//tp exe ·��.
	CString strPackPath;
	GetDlgItemText(IDC_EDIT_PACKPATH, strPackPath);

	// ���Ŀ¼
	CString strOutPath;
	GetDlgItemText(IDC_EDIT_OUTPUT, strOutPath);

	//runpath Ŀ¼.
	CString strRunPathDir;
	GetDlgItemText(IDC_EDIT_RUNPATHDIR,strRunPathDir);


	//��Ŀ¼.
	CButton *pBtn = (CButton*)(GetDlgItem(IDC_CHECK_CHILDDIR));
	bool bChildDir = pBtn->GetCheck() == TRUE;


	CImgToolComm::GetSignleInstance()->setWorkDir(strPackPath);


	//imgresxml 
	CImgResXmlOpreate imgResXmlLast; //�ϴε�XML�ļ�.(�������ɵ�XML�е����ļ�.)
	CImgResXmlOpreate imgResXmlCur;  //��ǰ��XML�ļ�.(Ŀǰrunpath�е��ļ�)

	imgResXmlCur.set_work_dir(strRunPathDir);
	imgResXmlLast.update_img_info();
	imgResXmlCur.set_lastxml_ptr(&imgResXmlLast);
	imgResXmlCur.set_tp_out_dir(strOutPath.GetBuffer());

	// ���ò���.
	CImgPack imgPack;
	imgPack.setMainDir(strPackPath.GetBuffer());

	imgPack.setOutPutDir(strOutPath.GetBuffer());

	imgPack.setTexturepackExePath(strExePath.GetBuffer());

	imgPack.setXmlPtr(&imgResXmlCur,&imgResXmlLast);

	bool bRes = imgPack.startPack(bChildDir);
	if(bRes)
	{
		//�ϲ���XML�� .
		const std::vector<Data>& vecResDir = imgPack.getPackDirVec();
		for (int nIndex = 0; nIndex < vecResDir.size();++ nIndex)
		{
			std::map<string, _stImgSetInfo> curSetInfo;
			imgResXmlCur.joinXmlFile(vecResDir[nIndex].c_str(),&curSetInfo);
			update_list(&m_mylistOutPutImgInfo,curSetInfo,-1);
		}
	}

	imgResXmlCur.SaveFile("");
	//���������Ϣ.
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
	// ͬ��.
}


void CimageResToolsDlg::OnClose()
{
	saveDlgItemText();
	CDialogEx::OnClose();
}


void CimageResToolsDlg::OnBnClickedButton2()
{
	// ����Ŀ¼��XML
	//runpath Ŀ¼.
	CString strRunPathDir;
	GetDlgItemText(IDC_EDIT_RUNPATHDIR, strRunPathDir);

	CImgResXmlOpreate imgResXmlCur;  //��ǰ��XML�ļ�.(Ŀǰrunpath�е��ļ�)

	imgResXmlCur.set_work_dir(strRunPathDir);

	imgResXmlCur.SaveFile("");
}
