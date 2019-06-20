// BinocularDlg.cpp : 实现文件
/*
注意 不建议在MFC中调用WINAPI的CreatThread函数创建线程，那种是底层方法，实际的MFC对线程有自己的管理体系，建议使用AFXcreatthread
*/
#include "stdafx.h"
#include "Binocular_V_1.h"
#include "BinocularDlg.h"
#include "afxdialogex.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

//CAboutDlg 对话框
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};
//关于界面
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBinocularDlg 对话框
CBinocularDlg::CBinocularDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BINOCULAR_V_1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	cam_online_num = 0;
}

CBinocularDlg::~CBinocularDlg()
{
	OnBnClickedBtnEnd();
	//关闭定时器
	KillTimer(CAM_FINDER_TIMER_ID);
	KillTimer(CAM_FRAMERATE_GET_ID);
	KillTimer(PROJ_FINDER_TIMER_ID);
	//关闭特殊区域
	DeleteCriticalSection(&Log_Protection);
	DeleteCriticalSection(&Proj_Protection);
}

void CBinocularDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_TRIGGER, _triger_type);
	DDX_IPAddress(pDX, IDC_IPADD_LEFT, _left_ip);
	DDX_IPAddress(pDX, IDC_IPADD_RIGHT, _right_ip);
	DDX_Control(pDX, IDC_EDITLOG, _log_ctrl);
	DDX_Control(pDX, IDC_STCRFRMRT, _right_frate);
	DDX_Control(pDX, IDC_STCLFRMRT, _left_frate);
	DDX_Control(pDX, IDC_EDT_EXPOREL, _left_expose_value);
	DDX_Control(pDX, IDC_EDT_EXPORER, _right_expose_value);
	DDX_Control(pDX, IDC_EDT_FRAMESETL, _frame_set_l);
	DDX_Control(pDX, IDC_EDT_FRAMESETR, _frame_set_r);
	DDX_Control(pDX, IDC_CHK_ISSAVING, _is_saving);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, _dir_path);
	DDX_Control(pDX, IDC_MFCEDITBROWSE2, _ini_path);
	DDX_Control(pDX, IDC_BTN_CONNECTPROJ, _btn_connectproj);
	DDX_Control(pDX, IDC_BTN_CLOSEPROJ, _btn_closeproj);
	DDX_Control(pDX, IDC_EDT_EXPOSEURPROJ, _edt_proj_exp);
	DDX_Control(pDX, IDC_EDT_PERIODPROJ, _edt_proj_prd);
	DDX_Control(pDX, IDC_EDT_CYCLE1, _cycle_1);
	DDX_Control(pDX, IDC_EDT_CYCLE2, _cycle_2);
	DDX_Control(pDX, IDC_EDT_CYCLE3, _cycle_3);
	DDX_Control(pDX, IDC_EDT_STEP1, _step_1);
	DDX_Control(pDX, IDC_EDT_STEP2, _step_2);
	DDX_Control(pDX, IDC_EDT_STEP3, _step_3);
	DDX_Control(pDX, IDC_CHECK1, _is_repeat);
	DDX_Control(pDX, IDC_BTN_STARTEXP, _btn_startexp);
	DDX_Control(pDX, IDC_BTN_UNWARP, _btn_unwarp);
	DDX_Control(pDX, IDC_BTN_CALC3D, _btn_calc3d);
	DDX_Control(pDX, IDC_BTN_STOPACQ, _btn_stopacq);
}

BEGIN_MESSAGE_MAP(CBinocularDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_CMB_TRIGGER, &CBinocularDlg::OnSelchangeCmbTrigger)
	ON_BN_CLICKED(IDC_BTN_SETLEFT, &CBinocularDlg::OnBnClickedBtnopenleft)
	ON_BN_CLICKED(IDC_BTNCLOSELEFT, &CBinocularDlg::OnBnClickedBtncloseleft)
	ON_BN_CLICKED(IDC_BTN_SETRIGHT, &CBinocularDlg::OnBnClickedBtnopenright)
	ON_BN_CLICKED(IDC_BTNCLOSERIGHT, &CBinocularDlg::OnBnClickedBtncloseright)
	ON_BN_CLICKED(IDC_BTN_STARTACQ, &CBinocularDlg::OnBnClickedBtnStartacq)
	ON_BN_CLICKED(IDC_BTN_STOPACQ, &CBinocularDlg::OnBnClickedBtnStopacq)
	ON_BN_CLICKED(IDC_BTN_SETEXPOSEL, &CBinocularDlg::OnBnClickedBtnSetexposel)
	ON_BN_CLICKED(IDC_BTN_SETEXPOSER, &CBinocularDlg::OnBnClickedBtnSetexposer)
	ON_BN_CLICKED(IDC_CHK_ISSAVING, &CBinocularDlg::OnBnClickedChkIssaving)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE1, &CBinocularDlg::OnEnChangeMfceditbrowse1)
	ON_BN_CLICKED(IDC_BTN_STARTEXP, &CBinocularDlg::OnBnClickedBtnStartexp)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE2, &CBinocularDlg::OnEnChangeMfceditbrowse2)
	ON_BN_CLICKED(IDC_BTN_CLOSEPROJ, &CBinocularDlg::OnBnClickedBtnCloseproj)
	ON_BN_CLICKED(IDC_BTN_CONNECTPROJ, &CBinocularDlg::OnBnClickedBtnConnectproj)
	ON_EN_CHANGE(IDC_EDT_EXPOSEURPROJ, &CBinocularDlg::OnEnChangeEdtExposeurproj)
	ON_EN_CHANGE(IDC_EDT_PERIODPROJ, &CBinocularDlg::OnEnChangeEdtPeriodproj)
	ON_EN_CHANGE(IDC_EDT_CYCLE1, &CBinocularDlg::OnEnChangeEdtCycle1)
	ON_EN_CHANGE(IDC_EDT_CYCLE2, &CBinocularDlg::OnEnChangeEdtCycle2)
	ON_EN_CHANGE(IDC_EDT_CYCLE3, &CBinocularDlg::OnEnChangeEdtCycle3)
	ON_EN_CHANGE(IDC_EDT_STEP1, &CBinocularDlg::OnEnChangeEdtStep1)
	ON_EN_CHANGE(IDC_EDT_STEP2, &CBinocularDlg::OnEnChangeEdtStep2)
	ON_EN_CHANGE(IDC_EDT_STEP3, &CBinocularDlg::OnEnChangeEdtStep3)
	ON_BN_CLICKED(IDC_CHECK1, &CBinocularDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BTN_END, &CBinocularDlg::OnBnClickedBtnEnd)
	ON_BN_CLICKED(IDC_BTN_UNWARP, &CBinocularDlg::OnBnClickedBtnUnwarp)
END_MESSAGE_MAP()


// CBinocularDlg 生成的代码

BOOL CBinocularDlg::OnInitDialog()
{
	//About界面的插入
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	//生成的代码
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_ini_path.EnableFileBrowseButton(_T(""), _T("ini Files(*.ini)|*.ini|All Files (*.*)|*.*||"));
	//所有Pv函数在使用前 需要添加PvInitialize()
	PvInitialize();

	_triger_type.SetCurSel(2);
	_is_saving.SetCheck(0);
	_frame_set_l.SetWindowText(_T("30"));
	_frame_set_r.SetWindowText(_T("30"));
	_left_expose_value.SetWindowText(_T("10000"));
	_right_expose_value.SetWindowText(_T("10000"));


	//IP初始化
	_left_ip = ntohl(inet_addr("192.168.1.10"));		
	_right_ip = ntohl(inet_addr("192.168.1.9"));
	
	strtgy.col_cycles = { 70,72,84 };
	strtgy.col_steps = { 10,12,12 };

	_cycle_1.SetWindowText(_T("70"));
	_cycle_2.SetWindowText(_T("72"));
	_cycle_3.SetWindowText(_T("84"));
	_step_1.SetWindowText(_T("10"));
	_step_2.SetWindowText(_T("12"));
	_step_3.SetWindowText(_T("12"));

	UpdateData(FALSE);

	//开启关键区域
	InitializeCriticalSection(&Log_Protection);
	InitializeCriticalSection(&Proj_Protection);

	//初始化投影仪
	projector = new Projector();
	
	_edt_proj_exp.SetWindowText(_T("25000"));
	_edt_proj_prd.SetWindowText(_T("25000"));

	//开启定时器
	SetTimer(CAM_FINDER_TIMER_ID, 1000, 0);
	SetTimer(CAM_FRAMERATE_GET_ID, 1000, 0);
	SetTimer(PROJ_FINDER_TIMER_ID, 1000, 0);
	SetTimer(CAM_SHOW_PIC_L, 50, 0);
	SetTimer(CAM_SHOW_PIC_R, 50, 0);

	append_log(string("Enviroment Init Finished.\r\n"));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CBinocularDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CBinocularDlg::OnPaint()
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

HCURSOR CBinocularDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//自己的程序
void CBinocularDlg::OnSelchangeCmbTrigger()
{
	//改变触发模式 直接设定了两个相机
	//先获得触发模式 和 曝光时间
	int triger_mode = _triger_type.GetCurSel();
	CString fixed_frame_rate_s;
	if (left_Camera)
	{
		if (triger_mode == 3)
		{
			_frame_set_l.GetWindowText(fixed_frame_rate_s);
			int fixed_frame_rate = _ttoi(fixed_frame_rate_s);
			left_Camera->FrameRate = fixed_frame_rate;
		}
		if (!left_Camera->ChangeTrigerMode(triger_mode))
		{
			append_log(string("Wrnning: Left Camera changed trigger failed."));
		}
		append_log(left_Camera->outLog);
	}
	if (right_Camera)
	{
		if (triger_mode == 3)
		{
			_frame_set_r.GetWindowText(fixed_frame_rate_s);
			int fixed_frame_rate = _ttoi(fixed_frame_rate_s);
			right_Camera->FrameRate = fixed_frame_rate;
		}
		if (!right_Camera->ChangeTrigerMode(triger_mode))
		{
			append_log(string("Wrnning: Right Camera changed trigger failed."));
		}
		append_log(right_Camera->outLog);
	}
}

//开启左侧相机
void CBinocularDlg::OnBnClickedBtnopenleft()
{
	//仅仅新建一个相机对象的实例 不采图 不过需要检测相机是否存在
	UpdateData(TRUE);
	if (!left_Camera)	//如果相机不存在
	{
		left_Camera = new Camera(htonl(_left_ip), "Left");
		left_Camera->isSaving = _is_saving.GetCheck();
		_dir_path.GetWindowText(left_Camera->filepath);
		if (!left_Camera->Open())
		{
			append_log(left_Camera->outLog);
			delete left_Camera;
			left_Camera = NULL;
		}
		else
		{
			append_log(left_Camera->outLog);
			OnSelchangeCmbTrigger();
			OnBnClickedBtnSetexposel();
		}
	}
	else if (htonl(_left_ip) != left_Camera->IpAddr)	//如果相机存在 但IP不是 那么停止过去的相机 建立新的相机
	{
		OnBnClickedBtncloseright();
		left_Camera = new Camera(htonl(_left_ip), "Left");
		left_Camera->isSaving = _is_saving.GetCheck();
		_dir_path.GetWindowText(left_Camera->filepath);
		if (!left_Camera->Open())
		{
			append_log(left_Camera->outLog);
			delete left_Camera;
			left_Camera = NULL;
		}
		else
		{
			append_log(left_Camera->outLog);
			OnSelchangeCmbTrigger();
			OnBnClickedBtnSetexposel();
		}
	}
	else
	{
		//IP对应的话 按钮无效
	}
}

//关闭左侧相机 
void CBinocularDlg::OnBnClickedBtncloseleft()
{
	//销毁一个相机的实例	如果相机在采图 调用相机析构函数 理应关闭
	if (left_Camera == NULL)
	{
		append_log(string("Left Camera has already been removed or not Open."));
	}
	else
	{
		left_Camera->Close();
		append_log(left_Camera->outLog);
		delete left_Camera;
		left_Camera = NULL;
	}
}

//开启右侧相机
void CBinocularDlg::OnBnClickedBtnopenright()
{
	//仅仅新建一个相机对象的实例 不采图 不过需要检测相机是否存在
	UpdateData(TRUE);
	if (!right_Camera)	//如果相机不存在
	{
		right_Camera = new Camera(htonl(_right_ip), "Right");
		right_Camera->isSaving = _is_saving.GetCheck();
		_dir_path.GetWindowTextA(right_Camera->filepath);
		if (!right_Camera->Open())
		{
			append_log(right_Camera->outLog);
			delete right_Camera;
			right_Camera = NULL;
		}
		else
		{
			append_log(right_Camera->outLog);
			OnSelchangeCmbTrigger();
			OnBnClickedBtnSetexposer();
		}
	}
	else if (htonl(_right_ip) != right_Camera->IpAddr)	//如果相机存在 但IP不是 那么停止过去的相机 建立新的相机
	{
		OnBnClickedBtncloseright();
		right_Camera = new Camera(htonl(_right_ip), "Right");
		right_Camera->isSaving = _is_saving.GetCheck();
		_dir_path.GetWindowTextA(right_Camera->filepath);
		if (!right_Camera->Open())
		{
			append_log(right_Camera->outLog);
			delete right_Camera;
			right_Camera = NULL;
		}
		else
		{
			append_log(right_Camera->outLog);
			OnSelchangeCmbTrigger();
			OnBnClickedBtnSetexposer();
		}
	}
	else
	{
		//IP对应的话 按钮无效 
	}
}

//关闭右侧相机
void CBinocularDlg::OnBnClickedBtncloseright()
{
	//销毁一个相机的实例	如果相机在采图 调用相机析构函数 理应关闭
	if (right_Camera == NULL)
	{
		append_log(string("Right Camera has already been removed or not Open."));
	}
	else
	{
		right_Camera->Close();	//相机的Close一定要释放所有资源
		append_log(right_Camera->outLog);
		delete right_Camera;
		right_Camera = NULL;
	}
}

//同时仅运行单线程来修改Log区的日志 防止显示混乱
void CBinocularDlg::append_log(string & log_data)
{
	EnterCriticalSection(&Log_Protection);
	//添加日志
	CString log;
	_log_ctrl.GetWindowText(log);
	log += log_data.c_str();
	log += "\r\n";							//自带回车

	int iCount = log.GetLength();
	_log_ctrl.SetRedraw(FALSE);
	_log_ctrl.SetWindowText(log);
	int iLine = _log_ctrl.GetLineCount();
	_log_ctrl.LineScroll(iLine, 0);
	_log_ctrl.SetSel(iCount, iCount);
	_log_ctrl.SetRedraw(TRUE);
	LeaveCriticalSection(&Log_Protection);
}

void CBinocularDlg::update_projector_status()
{
	/*处理八个按钮的结果*/
	if (projector->is_connected)
	{
		((CButton*)GetDlgItem(IDC_RAD_PROJISCONNECT))->SetCheck(true);
		if (projector->is_standby)
		{
			((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(true);	//待机
			((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(false);	//视频模式
			((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(false);		//Pattern模式 只有在这个模式下，下述的状态才有效

			((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//正在投影
			((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//暂停状态
			((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//停止状态
			((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//验证完毕状态
		}
		else
		{
			if (projector->SLmode == false)
			{
				((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(false);	//待机
				((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(true);	//视频模式
				((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(false);		//Pattern模式 只有在这个模式下，下述的状态才有效

				((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//正在投影
				((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//暂停状态
				((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//停止状态
				((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//验证完毕状态
			}
			else
			{
				((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(false);	//待机
				((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(false);	//视频模式
				((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(true);		//Pattern模式 只有在这个模式下，下述的状态才有效
				
				projector->GetSeqStatus();
				if (projector->action == 0)
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//正在投影
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//暂停状态
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(true);		//停止状态
				}
				else if (projector->action == 1)
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//正在投影
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(true);		//暂停状态
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//停止状态
				}
				else if (projector->action == 2)
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(true);		//正在投影
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//暂停状态
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//停止状态
				}
				else
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//正在投影
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//暂停状态
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//停止状态
				}

				if (projector->is_validate)
				{
					((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(true);		//验证完毕状态
				}
				else
				{
					((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//验证完毕状态
				}
			}
		}

	}
	else
	{
		((CButton*)GetDlgItem(IDC_RAD_PROJISCONNECT))->SetCheck(false);	//连接
		((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(false);	//待机
		((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(false);	//视频模式
		((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(false);		//Pattern模式 只有在这个模式下，下述的状态才有效

		((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//正在投影
		((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//暂停状态
		((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//停止状态
		((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//验证完毕状态
	}
}

void CBinocularDlg::update_show(Camera* pt_cam, UINT ID)
{
	if (pt_cam == NULL)
		return;
	else
	{
		if (!pt_cam->isStreaming)
			return;
		else
		{
			Mat data;
			pt_cam->GetImage(data);
			CRect rect;

			GetDlgItem(ID)->GetClientRect(&rect);
			//调整图像宽度为4的倍数
			if (rect.Width() % 4 != 0)
			{
				rect.SetRect(rect.left, rect.top, rect.left + (rect.Width() + 3) / 4 * 4, rect.bottom);
				GetDlgItem(ID)->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOMOVE);
			}

			cv::resize(data, data, cv::Size(rect.Width(), rect.Height()),0,0,INTER_NEAREST);  //使图像适应控件大小 这是一个十分费时的操作

			unsigned int m_buffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
			BITMAPINFO* m_bmi = (BITMAPINFO*)m_buffer;
			BITMAPINFOHEADER* m_bmih = &(m_bmi->bmiHeader);

			memset(m_bmih, 0, sizeof(*m_bmih));
			m_bmih->biSize = sizeof(BITMAPINFOHEADER);
			m_bmih->biWidth = data.cols;   //必须为4的倍数
			m_bmih->biHeight = -data.rows; //在自下而上的位图中 高度为负
			m_bmih->biPlanes = 1;
			m_bmih->biCompression = BI_RGB;
			m_bmih->biBitCount = 8 * data.channels();

			if (data.channels() == 1)  //当图像为灰度图像时需要设置调色板颜色
			{
				for (int i = 0; i < 256; i++)
				{
					m_bmi->bmiColors[i].rgbBlue = i;
					m_bmi->bmiColors[i].rgbGreen = i;
					m_bmi->bmiColors[i].rgbRed = i;
					m_bmi->bmiColors[i].rgbReserved = 0;
				}
			}

			CDC *pDC = GetDlgItem(ID)->GetDC();
			::StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, rect.Width(), rect.Height(), data.data, (BITMAPINFO*)m_bmi, DIB_RGB_COLORS, SRCCOPY);
			ReleaseDC(pDC);
		}
	}
}

unsigned long FindCamAddLog(string& Out)
{
	Out.clear();
	//找相机
	tPvCameraInfo list[MAX_CAMERA_NUM];
	unsigned long numCams;
	numCams = PvCameraList(list, MAX_CAMERA_NUM, NULL);
	if (numCams < 10)
	{
		numCams += PvCameraListUnreachable(&list[numCams],MAX_CAMERA_NUM - numCams,NULL);
	}
	tPvIpSettings ipSetting;
	tPvErr Err;
	struct in_addr address;

	for (unsigned int i = 0; i < numCams; i++)
	{
		Err = PvCameraIpSettingsGet(list[i].UniqueId, &ipSetting);
		if (Err == ePvErrSuccess)
		{
			address.S_un.S_addr = ipSetting.CurrentIpAddress;
			Out += "Camera " + to_string(i) + ": IP(" + inet_ntoa(address)+")\r\n";
		}
		else
		{
			Out += "Camera" + to_string(i) + ": IP unavailiable";
		}
	}
	Out += "Have " + to_string(numCams) + " Cameras Founded.";
	return numCams;
}

void CBinocularDlg::OnTimer(UINT_PTR uId)
{
	switch (uId)
	{
	case CAM_FINDER_TIMER_ID:
	{
		unsigned long number_now = FindCamAddLog(log_buffer);
		if (cam_online_num != number_now)
		{
			append_log(log_buffer);
			log_buffer.clear();
			cam_online_num = number_now;
		}
		break;
	}
	case CAM_FRAMERATE_GET_ID: //可以用来更新帧率
	{
		int fram_rate = 0;
		if (left_Camera != NULL)
		{
			fram_rate = left_Camera->FrameCount - frmcnt_buffer_l;
			frmcnt_buffer_l = left_Camera->FrameCount;
			_left_frate.SetWindowText(to_string(fram_rate).c_str());
		}
		else
		{
			frmcnt_buffer_l = 0;
			_left_frate.SetWindowText("N/A");
		}
		if (right_Camera != NULL)
		{
			int fram_rate = right_Camera->FrameCount - frmcnt_buffer_r;
			frmcnt_buffer_r = right_Camera->FrameCount;
			_right_frate.SetWindowText(to_string(fram_rate).c_str());
		}
		else
		{
			frmcnt_buffer_r = 0;
			_right_frate.SetWindowText("N/A");
		}
		break;
	}
	case PROJ_FINDER_TIMER_ID:
	{
		if (projector)
		{
			bool old_state = projector->is_connected;
			projector->Discover_Timer_Call();
			bool new_state = projector->is_connected;
			if (old_state != new_state)
				append_log(projector->getLog());
			else
				projector->getLog();
			update_projector_status();
			_btn_closeproj.EnableWindow(true);
			_btn_connectproj.EnableWindow(false);
		}
		else
		{
			_btn_closeproj.EnableWindow(false);
			_btn_connectproj.EnableWindow(true);
		}
		break;
	}
	case CAM_SHOW_PIC_L:
	{
		if (left_Camera)
		{
			if (left_Camera->isStreaming)
			{
				update_show(left_Camera, IDC_IMGSHOWL);
			}
		}
		break;
	}
	case CAM_SHOW_PIC_R:
	{
		if (right_Camera)
		{
			if (right_Camera->isStreaming)
			{
				update_show(right_Camera, IDC_IMGSHOWR);
			}
		}
		break;
	}
	/*
		可以添加其他的时钟函数
	*/
	default:
		break;
	}
}

//初始化采集图像 调用完成后 相机处在拍摄状态 投影仪初始化完成 并等待play 

//此条设定完成之后，禁止更新一些重要的参数：投影仪的相关设定
void CBinocularDlg::OnBnClickedBtnStartacq()
{
	// TODO: 在此添加控件通知处理程序代码
	//先停止过去的测量
	OnBnClickedBtnStopacq();
	OnBnClickedBtnEnd();

	//再开始新的测量过程
	if (projector->exposure_time < 230 || projector->period_time < 230 || projector->ini_file_path == "")
	{
		append_log(string("Error: Projector do not have enough paras.(exp/prd time, ini file path)"));
		//如果 投影仪不在投影状态 禁用下列的按钮
		if (projector->action != 2)
		{
			_btn_startexp.EnableWindow(false);
			_btn_unwarp.EnableWindow(false);
			_btn_calc3d.EnableWindow(false);
			_btn_stopacq.EnableWindow(false);
		}
		return;
	}
	if (projector->Projector_Init() == false)
	{
		append_log(projector->getLog());
		//如果 投影仪不在投影状态 禁用下列的按钮
		if (projector->action != 2)
		{
			_btn_startexp.EnableWindow(false);
			_btn_unwarp.EnableWindow(false);
			_btn_calc3d.EnableWindow(false);
			_btn_stopacq.EnableWindow(false);
		}
		return;
	}
	append_log(projector->getLog());
	
	int size = strtgy.col_steps[0] + strtgy.col_steps[1] + strtgy.col_steps[2];

	if (size != projector->ini_LutEntriesNum)
	{
		append_log(string("Projector pattern number not match this setting."));
		//如果 投影仪不在投影状态 禁用下列的按钮
		if (projector->action != 2)
		{
			_btn_startexp.EnableWindow(false);
			_btn_unwarp.EnableWindow(false);
			_btn_calc3d.EnableWindow(false);
			_btn_stopacq.EnableWindow(false);
		}
		return;
	}

	
	//为相机创建一个自己的线程 这个线程仅仅用来开启相机
	if (left_Camera)
	{
		if (left_Camera->isStreaming == false)
		{
			//新建用于交互的数据类型
			if (buffer_left != NULL)
				delete[] buffer_left;
			buffer_left = NULL;

			if (left_buffer_ready != NULL)
				delete[] left_buffer_ready;
			left_buffer_ready = NULL;

			//一般来说，我们的程序在退出整体测量过程的时候，这里已经是清空的了
			/*if (left_buffer_cs != NULL)
				delete[] left_buffer_cs;
			left_buffer_cs = NULL;*/

			left_Camera->buffer_size = size;

			buffer_left = new cv::Mat[size];
			left_Camera->buffer = buffer_left;
			
			left_buffer_ready = new bool[size]();
			left_Camera->buffer_ready = left_buffer_ready;

			left_buffer_cs = new CRITICAL_SECTION[size];
			for (int i = 0; i < size; i++)
			{
				InitializeCriticalSection(&(left_buffer_cs[i]));
			}
			left_Camera->buffer_cs = left_buffer_cs;

			left_Camera->proj = projector;
			left_Camera->proj_protect = &Proj_Protection;
			left_need_unwrap = true;
		}
	}
	else
	{
		left_need_unwrap = false;
	}
	if (right_Camera)
	{
		if (right_Camera->isStreaming == false)
		{
			if (buffer_right != NULL)
				delete[] buffer_right;
			buffer_right = NULL;

			if (right_buffer_ready != NULL)
				delete[] right_buffer_ready;
			right_buffer_ready = NULL;

			//if (right_buffer_cs != NULL)
			//	delete[] right_buffer_cs;
			//right_buffer_cs = NULL;

			right_Camera->buffer_size = size;

			buffer_right = new cv::Mat[size];
			right_Camera->buffer = buffer_right;

			right_buffer_ready = new bool[size]();
			right_Camera->buffer_ready = right_buffer_ready;

			right_buffer_cs = new CRITICAL_SECTION[size];
			for (int i = 0; i < size; i++)
			{
				InitializeCriticalSection(&(right_buffer_cs[i]));
			}
			right_Camera->buffer_cs = right_buffer_cs;

			right_Camera->proj = projector;
			right_Camera->proj_protect = &Proj_Protection;
			right_need_unwrap = true;
		}
	}
	else
	{
		right_need_unwrap = false;
	}

	_btn_startexp.EnableWindow(true);
	_btn_unwarp.EnableWindow(true);
	_btn_calc3d.EnableWindow(true);
	_btn_stopacq.EnableWindow(true);
	append_log(string("Mesurement setting init commplete."));
}

//开始曝光拍摄（也许是运行的过程中结果乱了，导致漏帧，需要重新来）

//注意，之前开始相机的时候，处于外触发等待的状态，这个函数之后，开始进行真正的采图

//一共触发几次，拍摄多少张图，每张图代表是第几个相位的第几张，都是一个结构体定的。
void CBinocularDlg::OnBnClickedBtnStartexp()
{
	// TODO: 在此添加控件通知处理程序代码
	//先开启相机 再开启投影仪
	projector->GetSeqStatus();
	Sleep(200);
	if (projector->action != 2)
	{
		if (left_Camera)
		{
			if (left_Camera->isStreaming == false)
			{
				AfxBeginThread(Left_ThreadCapture, this);
			}
			else
			{
				left_Camera->CloseCapture();
				append_log(left_Camera->outLog);
				AfxBeginThread(Left_ThreadCapture, this);
			}
		}
		if (right_Camera)
		{
			if (right_Camera->isStreaming == false)
			{
				AfxBeginThread(Right_ThreadCapture, this);
			}
			else
			{
				right_Camera->CloseCapture();
				append_log(right_Camera->outLog);
				AfxBeginThread(Right_ThreadCapture, this);
			}
		}
		Sleep(200);
		projector->Play();
	}
}

//停止采集图像 由于相机是采用投影仪的触发时钟去运作，我们只需要让投影仪stop就可以了 这样相机也会处于一种暂停待命的模式，并且需要更新
void CBinocularDlg::OnBnClickedBtnStopacq()
{
	// TODO: 在此添加控件通知处理程序代码
	//先关闭相机 
	if (left_Camera)
	{
		if (left_Camera->isStreaming)
		{
			left_Camera->CloseCapture();
			append_log(left_Camera->outLog);
		}
	}
	if (right_Camera)
	{
		if (right_Camera->isStreaming)
		{
			right_Camera->CloseCapture();
			append_log(right_Camera->outLog);
		}
	}

	//再关闭投影仪
	if (projector->Stop() == false)
		append_log(projector->getLog());
	else
		append_log(projector->getLog());
}

//停止测量过程 将除去所有的测量记录 需要从头进行初始化。
void CBinocularDlg::OnBnClickedBtnEnd()
{
	OnBnClickedBtnStopacq();
	// TODO: 在此添加控件通知处理程序代码

	//先让重建的线程停止
	if (left_is_unwrap == true)
	{
		left_need_unwrap = false;
		WaitForSingleObject(left_unwrap_h,INFINITY);		//直到线程退出才停止
		left_is_unwrap = false;
	}
	if (right_is_unwrap == true)
	{
		right_need_unwrap = false;
		WaitForSingleObject(right_unwrap_h, INFINITY);		//直到线程退出才停止
		right_is_unwrap = false;
	}
	
	//然后再关闭相机
	if (left_Camera)
	{
		if (left_Camera->isStreaming)
		{
			left_Camera->CloseCapture();
			append_log(left_Camera->outLog);
		}
		delete[] buffer_left;
		left_Camera->buffer = NULL;
		buffer_left = NULL;

		delete[] left_buffer_ready;
		left_Camera->buffer_ready = NULL;
		left_buffer_ready = NULL;
		//手动关闭并释放空间
		for (int i = 0; i < left_Camera->buffer_size; i++)
		{
			DeleteCriticalSection(&left_buffer_cs[i]);
		}
		delete[] left_buffer_cs;
		left_buffer_cs = NULL;
		left_Camera->buffer_cs = NULL;

		left_Camera->buffer_size = 0;
		buffer_left = NULL;

	}
	if (right_Camera)
	{
		if (right_Camera->isStreaming)
		{
			right_Camera->CloseCapture();
			append_log(right_Camera->outLog);
		}
		delete[] buffer_right;
		right_Camera->buffer = NULL;
		buffer_right = NULL;

		delete[] right_buffer_ready;
		right_Camera->buffer_ready = NULL;
		right_buffer_ready = NULL;
		//手动关闭并释放空间
		for (int i = 0; i < right_Camera->buffer_size; i++)
		{
			DeleteCriticalSection(&right_buffer_cs[i]);
		}
		delete[] right_buffer_cs;
		right_buffer_cs = NULL;
		right_Camera->buffer_cs = NULL;

		right_Camera->buffer_size = 0;
		buffer_right = NULL;
	}

	_btn_startexp.EnableWindow(false);
	_btn_unwarp.EnableWindow(false);
	_btn_calc3d.EnableWindow(false);
	_btn_stopacq.EnableWindow(false);
}

UINT Left_ThreadCapture(LPVOID lpParam)
{
	CBinocularDlg *dlg = (CBinocularDlg*)lpParam;
	bool bCapture;
	bCapture = dlg->left_Camera->StartCapture();//调用开始采集函数
	dlg->append_log(dlg->left_Camera->outLog);
	return bCapture;
}

UINT Right_ThreadCapture(LPVOID lpParam)
{
	CBinocularDlg *dlg = (CBinocularDlg*)lpParam;
	bool bCapture;
	bCapture = dlg->right_Camera->StartCapture();//调用开始采集函数
	dlg->append_log(dlg->right_Camera->outLog);
	return bCapture;
}


//改变曝光值
void CBinocularDlg::OnBnClickedBtnSetexposel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (left_Camera)
	{
		CString expose_s;
		int expose_v;
		_left_expose_value.GetWindowText(expose_s);
		expose_v = _ttoi(expose_s);
		if (!left_Camera->ChangeExposeValue(expose_v))
		{
			append_log(string("Wrnning: Left Camera changed expose val failed."));
		};
		append_log(left_Camera->outLog);
	}
}

void CBinocularDlg::OnBnClickedBtnSetexposer()
{
	// TODO: 在此添加控件通知处理程序代码
	if (right_Camera)
	{
		CString expose_s;
		int expose_v;
		_right_expose_value.GetWindowText(expose_s);
		expose_v = _ttoi(expose_s);
		if (!right_Camera->ChangeExposeValue(expose_v))
		{
			append_log(string("Wrnning: Right Camera changed expose val failed."));
		};
		append_log(right_Camera->outLog);
	}
}

void CBinocularDlg::OnBnClickedChkIssaving()
{
	// TODO: 在此添加控件通知处理程序代码
	
	CString path;
	_dir_path.GetWindowText(path);
	int btn_state = _is_saving.GetCheck();
	if (left_Camera)
	{
		if (btn_state == BST_UNCHECKED)
			left_Camera->isSaving = false;
		else if (btn_state == BST_CHECKED)
		{
			if (path == "")
			{
				append_log(string("No path avaliable"));
				return;
			}
			left_Camera->isSaving = true;
		}
	}
	if (right_Camera)
	{
		if (btn_state == BST_UNCHECKED)
			right_Camera->isSaving = false;
		else if (btn_state == BST_CHECKED)
		{
			if (path == "")
			{
				append_log(string("No path avaliable"));
				return;
			}
			right_Camera->isSaving = true;
		}
	}
}

void CBinocularDlg::OnEnChangeMfceditbrowse1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	//幅值给现在存在的相机
	if (left_Camera)
		_dir_path.GetWindowText(left_Camera->filepath);
	if (right_Camera)
		_dir_path.GetWindowText(right_Camera->filepath);

	//不论相机是否存在 先创建一个文件夹
	CString Path;
	_dir_path.GetWindowText(Path);
	//在本文件夹下检测 是否有以相机名字命名的两个文件夹，如果有就采用。
	CString left_Path = Path + "\\Left";
	CString right_Path = Path + "\\Right";

	//如果没有 新建
	if (_access((LPSTR)(LPCTSTR)left_Path, 0) != 0)
	{
		_mkdir((LPSTR)(LPCTSTR)left_Path);
	}
	if (_access((LPSTR)(LPCTSTR)right_Path, 0) != 0)
	{
		_mkdir((LPSTR)(LPCTSTR)right_Path);
	}
}



void CBinocularDlg::OnEnChangeMfceditbrowse2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	
	// TODO:  在此添加控件通知处理程序代码
	CString _ini_cstring;
	_ini_path.GetWindowTextA(_ini_cstring);
	projector->ini_file_path = _ini_cstring.GetBuffer(0);
}

void CBinocularDlg::OnBnClickedBtnCloseproj()
{
	// TODO: 在此添加控件通知处理程序代码
	delete projector;
	projector = NULL;
}

void CBinocularDlg::OnBnClickedBtnConnectproj()
{
	// TODO: 在此添加控件通知处理程序代码
	projector = new Projector();
}

/*投影仪曝光时间*/
void CBinocularDlg::OnEnChangeEdtExposeurproj()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_exposure_time;
	_edt_proj_exp.GetWindowText(cstr_exposure_time);
	projector->exposure_time = _ttoi(cstr_exposure_time);
}

void CBinocularDlg::OnEnChangeEdtPeriodproj()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_proid_time;
	_edt_proj_prd.GetWindowText(cstr_proid_time);
	projector->period_time = _ttoi(cstr_proid_time);
}


/*条纹周期性和步长性设置*/
void CBinocularDlg::OnEnChangeEdtCycle1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_cycle;
	_cycle_1.GetWindowText(cstr_cycle);
	strtgy.col_cycles[0] = _ttoi(cstr_cycle);
}

void CBinocularDlg::OnEnChangeEdtCycle2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_cycle;
	_cycle_2.GetWindowText(cstr_cycle);
	strtgy.col_cycles[1] = _ttoi(cstr_cycle);
}

void CBinocularDlg::OnEnChangeEdtCycle3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_cycle;
	_cycle_3.GetWindowText(cstr_cycle);
	strtgy.col_cycles[2] = _ttoi(cstr_cycle);
}

void CBinocularDlg::OnEnChangeEdtStep1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_step;
	_step_1.GetWindowText(cstr_step);
	strtgy.col_steps[0] = _ttoi(cstr_step);
}

void CBinocularDlg::OnEnChangeEdtStep2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_step;
	_step_2.GetWindowText(cstr_step);
	strtgy.col_steps[1] = _ttoi(cstr_step);
}

void CBinocularDlg::OnEnChangeEdtStep3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	CString cstr_step;
	_step_3.GetWindowText(cstr_step);
	strtgy.col_steps[2] = _ttoi(cstr_step);
}

void CBinocularDlg::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (left_Camera)
	{
		left_Camera->isRepeat = _is_repeat.GetCheck();
	}
	if (right_Camera)
	{
		right_Camera->isRepeat = _is_repeat.GetCheck();
	}
}


//工作者线程函数 目前只重建左方相机
UINT Left_ThreadUnwarp(LPVOID lpParam)
{
	CBinocularDlg *dlg = (CBinocularDlg*)lpParam;
	int pic_count = 1;
	unsigned int all_size = dlg->strtgy.col_steps[0] + dlg->strtgy.col_steps[1] + dlg->strtgy.col_steps[2];		//图样总大小
	int count = 0;		//用于表示到底用了多少个波长 单频有单频的解相方法，多频有多频的解相方法 max = 3 
	for (int i = 0; i < dlg->strtgy.col_steps.size(); i++)
	{
		if (dlg->strtgy.col_steps[i] != 0)
		{
			count++;
		}	
	}

	vector<vector<cv::Mat>> func_img_buffer;		//函数内部的图像接收 这个类仅作为图像接口 我们不能拿动态更新的图像来做 这要求我们需要采用它来实现
	vector<cv::Mat> func_wrap_phase;				//最多接受3波长
	cv::Mat func_unwrap_phase;						//最后的绝对相位结果

	func_wrap_phase.resize(3);
	func_img_buffer.resize(3);						
	for (int i = 0; i < count; i++)
	{
		func_img_buffer[i].resize(dlg->strtgy.col_steps[i]);
	}
	bool cycle_pattern_changed[3];
	cycle_pattern_changed[0] = false;
	cycle_pattern_changed[1] = false;
	cycle_pattern_changed[2] = false;

	while (dlg->left_need_unwrap)
	{
		//首先从类中读取图像 检验该图像是否已经被读取，并且注意线程安全性（意图：尽量少干涉采集过程）
		for (int i = 0; i < all_size; i++)
		{		
			if (dlg->left_buffer_ready[i] == true)	//如果这一帧图像准备好了 判断它是谁的 存图，并且将需要计算包裹相位的结构置位，重建相位。
			{
				EnterCriticalSection(&(dlg->left_buffer_cs[i]));
				if (i >= 0 && i < dlg->strtgy.col_steps[0])
				{
					func_img_buffer[0][i] = dlg->buffer_left[i].clone();
					cycle_pattern_changed[0] = true;
				}
				if (i >= dlg->strtgy.col_steps[0] && i < dlg->strtgy.col_steps[0] + dlg->strtgy.col_steps[1])
				{
					func_img_buffer[1][i - dlg->strtgy.col_steps[0]] = dlg->buffer_left[i].clone();
					cycle_pattern_changed[1] = true;
				}
				if (i >= dlg->strtgy.col_steps[0] + dlg->strtgy.col_steps[1] && i < all_size)
				{
					func_img_buffer[2][i - dlg->strtgy.col_steps[0] - dlg->strtgy.col_steps[1]] = dlg->buffer_left[i].clone();
					cycle_pattern_changed[2] = true;
				}	
				LeaveCriticalSection(&(dlg->left_buffer_cs[i]));
			}
		}
		//接着针对其计算包裹相位，注意，对于已经重建相位的图像，我们不需要重新重建。从而提高本线程的处理速度 
		//注意 需要判断是否每个存储空间里都有图了
		if (cycle_pattern_changed[0])
		{
			bool _calculate_wrap = true;
			for (int i = 0; i < func_img_buffer[0].size(); i++)
			{
				if (func_img_buffer[0][i].empty())
				{
					_calculate_wrap = false;
					break;
				}
			}
			if (_calculate_wrap)
			{
				Get_Wrap_Phase_Steps(func_img_buffer[0], func_wrap_phase[0]);
				cycle_pattern_changed[0] = false;
			}
		}
		if (cycle_pattern_changed[1])
		{
			bool _calculate_wrap = true;
			for (int i = 0; i < func_img_buffer[1].size(); i++)
			{
				if (func_img_buffer[1][i].empty())
				{
					_calculate_wrap = false;
					break;
				}
			}
			if (_calculate_wrap)
			{
				Get_Wrap_Phase_Steps(func_img_buffer[1], func_wrap_phase[1]);
				cycle_pattern_changed[1] = false;
			}
		}
		if (cycle_pattern_changed[2])
		{
			bool _calculate_wrap = true;
			for (int i = 0; i < func_img_buffer[2].size(); i++)
			{
				if (func_img_buffer[2][i].empty())
				{
					_calculate_wrap = false;
					break;
				}
			}
			if (_calculate_wrap)
			{
				Get_Wrap_Phase_Steps(func_img_buffer[2], func_wrap_phase[2]);
				cycle_pattern_changed[2] = false;
			}
		}
		
		//最后 计算解包裹相位 注意 这里需要判断到底我们采用了是几张图像。同样 需要判断里面是否有数据
		bool _calculate_unwrap = true;
		for (int i = 0; i < dlg->strtgy.col_steps.size(); i++)
		{
			if (dlg->strtgy.col_steps[i] != 0)
			{
				if (func_wrap_phase[i].empty())
					_calculate_unwrap = false;
			}
		}
		if (_calculate_unwrap)
		{
			if (count == 3)
				Get_Absolute_Phase_3(func_wrap_phase, dlg->strtgy.col_cycles, func_unwrap_phase);
			if (count == 2)
				Get_Absolute_Phase_2(func_wrap_phase, dlg->strtgy.col_cycles, func_unwrap_phase);
			if (count == 1)
				//单波长解相方法，研发中。。。
				;

			//最后将我们的图像存储下来
			if (dlg->_is_saving.GetCheck() == BST_CHECKED)
			{
				Mat save_img;
				//func_unwrap_phase.convertTo(save_img, CV_8UC1, round(1824 / dlg->strtgy.col_cycles[0]/CV_PI/2) - 1);
				func_unwrap_phase.convertTo(save_img, CV_8UC1, 2);
				CString Path;
				dlg->_dir_path.GetWindowText(Path);
				char file_name[50];
				sprintf_s(file_name, "unwrap_phase_%05d.bmp", pic_count++);
				string full_name = Path + "\\" + file_name;
				imwrite(full_name, save_img);
			}
		}

	}
	return 0;
}

//工作者线程函数
UINT Right_ThreadUnwarp(LPVOID lpParam)
{
	CBinocularDlg *dlg = (CBinocularDlg*)lpParam;
	//首先 获得一些最基本的数据
	unsigned int all_size = dlg->strtgy.col_steps[0] + dlg->strtgy.col_steps[1] + dlg->strtgy.col_steps[2];
	int count = 0;		//用于表示到底用了多少个波长 单频有单频的解相方法，多频有多频的解相方法
	for (int i = 0; i < dlg->strtgy.col_steps.size(); i++)
	{
		if (dlg->strtgy.col_steps[i] != 0)
			count++;
	}

	while (dlg->right_need_unwrap)
	{

	}
	return 0;
}

void CBinocularDlg::OnBnClickedBtnUnwarp()
{
	// TODO: 在此添加控件通知处理程序代码
	//开辟两个线程来计算结果，并且需要把计算出来的结果进行显示 好吧我承认我放弃了。。。不显示了，改为存图
	//如果开发新的一版，绝对不会采用这种SB形式，做一个对话框调用一个对话框，从而实现效果
	//也许我可以直接调用每个相机的显示窗口来显示相位效果。。？ 并且一定要用VIMBA!!!!! 这个PVAPI用的我快死了
	//投影仪官方只有这个垃圾可以使用，那就先自己写了一个projector类 用这个吧（微笑）
	if (left_need_unwrap)
	{
		left_unwrap_h = AfxBeginThread(Left_ThreadUnwarp, this);
		left_is_unwrap = true;
	}
	//if (right_need_unwrap)
	//{
	//	right_unwrap_h = AfxBeginThread(Right_ThreadUnwarp, this);
	//	right_is_unwrap = true;
	//}
	//	
}
