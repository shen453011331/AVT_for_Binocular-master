// BinocularDlg.cpp : 实现文件
#include "stdafx.h"
#include "Binocular_V_1.h"
#include "BinocularDlg.h"
#include "afxdialogex.h"

#include <string>
#include <thread>

#include <io.h>
#include <direct.h>

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
	//关闭定时器
	KillTimer(CAM_FINDER_TIMER_ID);
	KillTimer(CAM_FRAMERATE_GET_ID);
	KillTimer(PROJ_FINDER_TIMER_ID);
	//关闭特殊区域
	DeleteCriticalSection(&Log_Protection);
}

void CBinocularDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_TRIGGER, _triger_type);
	DDX_Text(pDX, IDC_EDT_EXPORE, _setted_exp_val);
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

	_triger_type.SetCurSel(0);
	_is_saving.SetCheck(0);
	_frame_set_l.SetWindowText(_T("30"));
	_frame_set_r.SetWindowText(_T("30"));
	_left_expose_value.SetWindowText(_T("5000"));
	_right_expose_value.SetWindowText(_T("5000"));

	_left_ip = ntohl(inet_addr("192.168.1.3"));		//IP地址转换 将四个点分十进制转成2进制数 用于显示用于开启相机
	_right_ip = ntohl(inet_addr("192.168.1.2"));
	_setted_exp_val.Format(_T("%d"),5000);
	UpdateData(FALSE);

	//开启关键区域
	InitializeCriticalSection(&Log_Protection);

	//初始化投影仪
	projector = new Projector();
	
	//开启定时器
	SetTimer(CAM_FINDER_TIMER_ID, 1000, 0);
	SetTimer(CAM_FRAMERATE_GET_ID, 1000, 0);
	SetTimer(PROJ_FINDER_TIMER_ID, 1000, 0);
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
	//先获得
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
		_dir_path.GetWindowTextA(left_Camera->filepath);
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
		}
			
	}
	else if (htonl(_left_ip) != left_Camera->IpAddr)	//如果相机存在 但IP不是 那么停止过去的相机 建立新的相机
	{
		OnBnClickedBtncloseright();
		left_Camera = new Camera(htonl(_left_ip), "Left");
		left_Camera->isSaving = _is_saving.GetCheck();
		_dir_path.GetWindowTextA(left_Camera->filepath);
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
		}
			
	}
	//IP对应的话 按钮无效
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
		}
	}
	//IP对应的话 按钮无效 
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
			bool new_state = projector->Discover_Timer_Call();
			if (old_state != new_state)
				append_log(projector->getLog());
			update_projector_status();
		}
	}
	/*
		可以添加其他的时钟函数
	*/
	default:
		break;
	}
}


//开始采集图像 调用完成后 相机处在拍摄状态 投影仪初始化完成 并等待play
void CBinocularDlg::OnBnClickedBtnStartacq()
{
	// TODO: 在此添加控件通知处理程序代码
	if (projector->Projector_Init() == false)
	{
		append_log(projector->getLog());
		return;
	}
	append_log(projector->getLog());
	//为相机创建一个自己的线程
	if (left_Camera)
	{
		if (left_Camera->isStreaming == false)
			left_thread = AfxBeginThread(Left_ThreadCapture, this);
	}
	if (right_Camera)
	{
		if (right_Camera->isStreaming == false)
			right_thread = AfxBeginThread(Right_ThreadCapture, this);
	}
}
//停止采集图像
void CBinocularDlg::OnBnClickedBtnStopacq()
{
	// TODO: 在此添加控件通知处理程序代码
	//先关闭投影仪 再关闭相机
	if (projector->Stop() == false)
	{
		append_log(projector->getLog());
		//return 
	}
	else
		append_log(projector->getLog());

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
	if (left_Camera)
	{
		left_Camera->isSaving = _is_saving.GetCheck();
	}
	if (right_Camera)
	{
		right_Camera->isSaving = _is_saving.GetCheck();
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
		_dir_path.GetWindowTextA(left_Camera->filepath);
	if (right_Camera)
		_dir_path.GetWindowTextA(right_Camera->filepath);

	//不论相机是否存在 先创建一个文件夹
	CString Path;
	_dir_path.GetWindowTextA(Path);
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


//开始曝光拍摄
//注意，之前开始相机的时候，处于外触发等待的状态，这个函数之后，开始进行真正的采图
//一共触发几次，拍摄多少张图，每张图代表是第几个相位的第几张，都是一个结构体定的。
void CBinocularDlg::OnBnClickedBtnStartexp()
{
	// TODO: 在此添加控件通知处理程序代码
	projector->Play();
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

