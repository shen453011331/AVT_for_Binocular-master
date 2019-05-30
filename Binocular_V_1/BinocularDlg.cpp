// BinocularDlg.cpp : ʵ���ļ�
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

//CAboutDlg �Ի���
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};
//���ڽ���
CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBinocularDlg �Ի���

CBinocularDlg::CBinocularDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BINOCULAR_V_1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	cam_online_num = 0;
}

CBinocularDlg::~CBinocularDlg()
{
	//�رն�ʱ��
	KillTimer(CAM_FINDER_TIMER_ID);
	KillTimer(CAM_FRAMERATE_GET_ID);
	KillTimer(PROJ_FINDER_TIMER_ID);
	//�ر���������
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
	DDX_Control(pDX, IDC_EDT_EXPOSEURPROJ, _edt_proj_exp);
	DDX_Control(pDX, IDC_EDT_PERIODPROJ, _edt_proj_prd);
	DDX_Control(pDX, IDC_EDT_CYCLE1, _cycle_1);
	DDX_Control(pDX, IDC_EDT_CYCLE2, _cycle_2);
	DDX_Control(pDX, IDC_EDT_CYCLE3, _cycle_3);
	DDX_Control(pDX, IDC_EDT_STEP1, _step_1);
	DDX_Control(pDX, IDC_EDT_STEP2, _step_2);
	DDX_Control(pDX, IDC_EDT_STEP3, _step_3);
	DDX_Control(pDX, IDC_CHECK1, _is_repeat);
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
END_MESSAGE_MAP()


// CBinocularDlg ���ɵĴ���

BOOL CBinocularDlg::OnInitDialog()
{
	//About����Ĳ���
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
	//���ɵĴ���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	_ini_path.EnableFileBrowseButton(_T(""), _T("ini Files(*.ini)|*.ini|All Files (*.*)|*.*||"));
	//����Pv������ʹ��ǰ ��Ҫ���PvInitialize()
	PvInitialize();

	_triger_type.SetCurSel(0);
	_is_saving.SetCheck(0);
	_frame_set_l.SetWindowText(_T("30"));
	_frame_set_r.SetWindowText(_T("30"));
	_left_expose_value.SetWindowText(_T("5000"));
	_right_expose_value.SetWindowText(_T("5000"));

	_left_ip = ntohl(inet_addr("192.168.1.3"));		//IP��ַת�� ���ĸ����ʮ����ת��2������ ������ʾ���ڿ������
	_right_ip = ntohl(inet_addr("192.168.1.2"));
	_setted_exp_val.Format(_T("%d"),5000);
	
	strtgy.col_cycles = { 70,72,84 };
	strtgy.col_steps = { 3,3,3 };

	_cycle_1.SetWindowText(_T("70"));
	_cycle_2.SetWindowText(_T("72"));
	_cycle_3.SetWindowText(_T("84"));
	_step_1.SetWindowText(_T("3"));
	_step_2.SetWindowText(_T("3"));
	_step_3.SetWindowText(_T("3"));
	UpdateData(FALSE);

	//�����ؼ�����
	InitializeCriticalSection(&Log_Protection);

	//��ʼ��ͶӰ��
	projector = new Projector();
	
	//������ʱ��
	SetTimer(CAM_FINDER_TIMER_ID, 1000, 0);
	SetTimer(CAM_FRAMERATE_GET_ID, 1000, 0);
	SetTimer(PROJ_FINDER_TIMER_ID, 1000, 0);
	append_log(string("Enviroment Init Finished.\r\n"));
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

HCURSOR CBinocularDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//�Լ��ĳ���
void CBinocularDlg::OnSelchangeCmbTrigger()
{
	//�ı䴥��ģʽ ֱ���趨���������
	//�Ȼ��
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

//����������
void CBinocularDlg::OnBnClickedBtnopenleft()
{
	//�����½�һ����������ʵ�� ����ͼ ������Ҫ�������Ƿ����
	UpdateData(TRUE);
	if (!left_Camera)	//������������
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
	else if (htonl(_left_ip) != left_Camera->IpAddr)	//���������� ��IP���� ��ôֹͣ��ȥ����� �����µ����
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
	//IP��Ӧ�Ļ� ��ť��Ч
}

//�ر������� 
void CBinocularDlg::OnBnClickedBtncloseleft()
{
	//����һ�������ʵ��	�������ڲ�ͼ ��������������� ��Ӧ�ر�
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

//�����Ҳ����
void CBinocularDlg::OnBnClickedBtnopenright()
{
	//�����½�һ����������ʵ�� ����ͼ ������Ҫ�������Ƿ����
	UpdateData(TRUE);
	if (!right_Camera)	//������������
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
	else if (htonl(_right_ip) != right_Camera->IpAddr)	//���������� ��IP���� ��ôֹͣ��ȥ����� �����µ����
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
	//IP��Ӧ�Ļ� ��ť��Ч 
}

//�ر��Ҳ����
void CBinocularDlg::OnBnClickedBtncloseright()
{
	//����һ�������ʵ��	�������ڲ�ͼ ��������������� ��Ӧ�ر�
	if (right_Camera == NULL)
	{
		append_log(string("Right Camera has already been removed or not Open."));
	}
	else
	{
		right_Camera->Close();	//�����Closeһ��Ҫ�ͷ�������Դ
		append_log(right_Camera->outLog);
		delete right_Camera;
		right_Camera = NULL;
	}
}

//ͬʱ�����е��߳����޸�Log������־ ��ֹ��ʾ����
void CBinocularDlg::append_log(string & log_data)
{
	EnterCriticalSection(&Log_Protection);
	//�����־
	CString log;
	_log_ctrl.GetWindowText(log);
	log += log_data.c_str();
	log += "\r\n";							//�Դ��س�

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
	/*����˸���ť�Ľ��*/
	if (projector->is_connected)
	{
		((CButton*)GetDlgItem(IDC_RAD_PROJISCONNECT))->SetCheck(true);
		if (projector->is_standby)
		{
			((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(true);	//����
			((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(false);	//��Ƶģʽ
			((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(false);		//Patternģʽ ֻ�������ģʽ�£�������״̬����Ч

			((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//����ͶӰ
			((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//��ͣ״̬
			((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//ֹͣ״̬
			((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//��֤���״̬
		}
		else
		{
			if (projector->SLmode == false)
			{
				((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(false);	//����
				((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(true);	//��Ƶģʽ
				((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(false);		//Patternģʽ ֻ�������ģʽ�£�������״̬����Ч

				((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//����ͶӰ
				((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//��ͣ״̬
				((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//ֹͣ״̬
				((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//��֤���״̬
			}
			else
			{
				((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(false);	//����
				((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(false);	//��Ƶģʽ
				((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(true);		//Patternģʽ ֻ�������ģʽ�£�������״̬����Ч
				
				projector->GetSeqStatus();
				if (projector->action == 0)
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//����ͶӰ
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//��ͣ״̬
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(true);		//ֹͣ״̬
				}
				else if (projector->action == 1)
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//����ͶӰ
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(true);		//��ͣ״̬
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//ֹͣ״̬
				}
				else if (projector->action == 2)
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(true);		//����ͶӰ
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//��ͣ״̬
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//ֹͣ״̬
				}
				else
				{
					((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//����ͶӰ
					((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//��ͣ״̬
					((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//ֹͣ״̬
				}

				if (projector->is_validate)
				{
					((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(true);		//��֤���״̬
				}
				else
				{
					((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//��֤���״̬
				}
			}
		}

	}
	else
	{
		((CButton*)GetDlgItem(IDC_RAD_PROJISCONNECT))->SetCheck(false);	//����
		((CButton*)GetDlgItem(IDC_RAD_PROJISPOWERBY))->SetCheck(false);	//����
		((CButton*)GetDlgItem(IDC_RAD_PROJISVIDEO))->SetCheck(false);	//��Ƶģʽ
		((CButton*)GetDlgItem(IDC_RAD_PROJISSEQ))->SetCheck(false);		//Patternģʽ ֻ�������ģʽ�£�������״̬����Ч

		((CButton*)GetDlgItem(IDC_RAD_ISRUNNING))->SetCheck(false);		//����ͶӰ
		((CButton*)GetDlgItem(IDC_RAD_ISPAUSE))->SetCheck(false);		//��ͣ״̬
		((CButton*)GetDlgItem(IDC_RAD_ISSTOP))->SetCheck(false);		//ֹͣ״̬
		((CButton*)GetDlgItem(IDC_RAD_VALIDPASS))->SetCheck(false);		//��֤���״̬
	}

	
}

unsigned long FindCamAddLog(string& Out)
{
	Out.clear();
	//�����
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
	case CAM_FRAMERATE_GET_ID: //������������֡��
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
			_btn_closeproj.EnableWindow(true);
			_btn_connectproj.EnableWindow(false);
		}
		else
		{
			_btn_closeproj.EnableWindow(false);
			_btn_connectproj.EnableWindow(true);
		}
	}
	/*
		�������������ʱ�Ӻ���
	*/
	default:
		break;
	}
}

//��ʼ�ɼ�ͼ�� ������ɺ� �����������״̬ ͶӰ�ǳ�ʼ����� ���ȴ�play
void CBinocularDlg::OnBnClickedBtnStartacq()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (projector->exposure_time < 230 || projector->period_time < 230 || projector->ini_file_path == "")
	{
		append_log(string("Error: Projector do not have enough paras.(exp/prd time, ini file path)"));
		return;
	}
	if (projector->Projector_Init() == false)
	{
		append_log(projector->getLog());
		return;
	}
	append_log(projector->getLog());
	
	int size = strtgy.col_steps[0] + strtgy.col_steps[1] + strtgy.col_steps[2];

	if (size != projector->ini_LutEntriesNum)
	{
		append_log(string("Projector pattern number not match this setting."));
		return;
	}
	//Ϊ�������һ���Լ����߳�
	if (left_Camera)
	{
		if (left_Camera->isStreaming == false)
		{
			if (buffer_left != NULL)
				delete[] buffer_left;
			buffer_left = NULL;
			buffer_left = new cv::Mat[size];
			left_Camera->buffer = buffer_left;
			left_Camera->buffer_size = size;
			left_thread = AfxBeginThread(Left_ThreadCapture, this);
		}
			
	}
	if (right_Camera)
	{
		if (right_Camera->isStreaming == false)
		{
			if (buffer_right != NULL)
				delete[] buffer_right;
			buffer_right = NULL;
			buffer_right = new cv::Mat[size];
			right_Camera->buffer = buffer_right;
			right_Camera->buffer_size = size;
			right_thread = AfxBeginThread(Right_ThreadCapture, this);
		}
	}
}
//ֹͣ�ɼ�ͼ��
void CBinocularDlg::OnBnClickedBtnStopacq()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//�ȹر�ͶӰ�� �ٹر����
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
			delete[] buffer_left;
			right_Camera->buffer = NULL;
			left_Camera->buffer_size = 0;
			buffer_left = NULL;
		}
	}
	if (right_Camera)
	{
		if (right_Camera->isStreaming)
		{
			right_Camera->CloseCapture();
			append_log(right_Camera->outLog);
			delete[] buffer_right;
			right_Camera->buffer = NULL;
			right_Camera->buffer_size = 0;
			buffer_right = NULL;
		}
	}
}

UINT Left_ThreadCapture(LPVOID lpParam)
{
	CBinocularDlg *dlg = (CBinocularDlg*)lpParam;
	bool bCapture;
	bCapture = dlg->left_Camera->StartCapture();//���ÿ�ʼ�ɼ�����
	dlg->append_log(dlg->left_Camera->outLog);
	return bCapture;
}

UINT Right_ThreadCapture(LPVOID lpParam)
{
	CBinocularDlg *dlg = (CBinocularDlg*)lpParam;
	bool bCapture;
	bCapture = dlg->right_Camera->StartCapture();//���ÿ�ʼ�ɼ�����
	dlg->append_log(dlg->right_Camera->outLog);
	return bCapture;
}

//�ı��ع�ֵ
void CBinocularDlg::OnBnClickedBtnSetexposel()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//��ֵ�����ڴ��ڵ����
	if (left_Camera)
		_dir_path.GetWindowTextA(left_Camera->filepath);
	if (right_Camera)
		_dir_path.GetWindowTextA(right_Camera->filepath);

	//��������Ƿ���� �ȴ���һ���ļ���
	CString Path;
	_dir_path.GetWindowTextA(Path);
	//�ڱ��ļ����¼�� �Ƿ�����������������������ļ��У�����оͲ��á�
	CString left_Path = Path + "\\Left";
	CString right_Path = Path + "\\Right";


	//���û�� �½�
	if (_access((LPSTR)(LPCTSTR)left_Path, 0) != 0)
	{
		_mkdir((LPSTR)(LPCTSTR)left_Path);
	}
	if (_access((LPSTR)(LPCTSTR)right_Path, 0) != 0)
	{
		_mkdir((LPSTR)(LPCTSTR)right_Path);
	}
}

//��ʼ�ع�����
//ע�⣬֮ǰ��ʼ�����ʱ�򣬴����ⴥ���ȴ���״̬���������֮�󣬿�ʼ���������Ĳ�ͼ
//һ���������Σ����������ͼ��ÿ��ͼ�����ǵڼ�����λ�ĵڼ��ţ�����һ���ṹ�嶨�ġ�
void CBinocularDlg::OnBnClickedBtnStartexp()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	projector->Play();
}

void CBinocularDlg::OnEnChangeMfceditbrowse2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
	
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString _ini_cstring;
	_ini_path.GetWindowTextA(_ini_cstring);
	projector->ini_file_path = _ini_cstring.GetBuffer(0);
}

void CBinocularDlg::OnBnClickedBtnCloseproj()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	delete projector;
	projector = NULL;
}

void CBinocularDlg::OnBnClickedBtnConnectproj()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	projector = new Projector();
}

/*ͶӰ���ع�ʱ��*/
void CBinocularDlg::OnEnChangeEdtExposeurproj()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_exposure_time;
	_edt_proj_exp.GetWindowText(cstr_exposure_time);
	projector->exposure_time = _ttoi(cstr_exposure_time);
}

void CBinocularDlg::OnEnChangeEdtPeriodproj()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_proid_time;
	_edt_proj_prd.GetWindowText(cstr_proid_time);
	projector->period_time = _ttoi(cstr_proid_time);
}


/*���������ԺͲ���������*/
void CBinocularDlg::OnEnChangeEdtCycle1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_cycle;
	_cycle_1.GetWindowText(cstr_cycle);
	strtgy.col_cycles[0] = _ttoi(cstr_cycle);
}

void CBinocularDlg::OnEnChangeEdtCycle2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_cycle;
	_cycle_2.GetWindowText(cstr_cycle);
	strtgy.col_cycles[1] = _ttoi(cstr_cycle);
}

void CBinocularDlg::OnEnChangeEdtCycle3()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_cycle;
	_cycle_3.GetWindowText(cstr_cycle);
	strtgy.col_cycles[2] = _ttoi(cstr_cycle);
}

void CBinocularDlg::OnEnChangeEdtStep1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_step;
	_step_1.GetWindowText(cstr_step);
	strtgy.col_steps[0] = _ttoi(cstr_step);
}

void CBinocularDlg::OnEnChangeEdtStep2()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_step;
	_step_2.GetWindowText(cstr_step);
	strtgy.col_steps[1] = _ttoi(cstr_step);
}

void CBinocularDlg::OnEnChangeEdtStep3()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString cstr_step;
	_step_3.GetWindowText(cstr_step);
	strtgy.col_steps[2] = _ttoi(cstr_step);
}


void CBinocularDlg::OnBnClickedCheck1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (left_Camera)
	{
		left_Camera->is_reapeat = _is_repeat.GetCheck();
	}
	if (right_Camera)
	{
		right_Camera->is_reapeat = _is_repeat.GetCheck();
	}
}
