
// BinocularDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#include "Camera.h"
#include "Projector.h"
#include "calculate_3D.h"

#include <string>
#include "afxeditbrowsectrl.h"

#define CAM_FINDER_TIMER_ID		1			//���ڶ�̬�����Ƿ����������/�γ���TIMER
#define CAM_FRAMERATE_GET_ID    2			//���ڶ�̬������ͼ��������TIMER
#define PROJ_FINDER_TIMER_ID	3			//���ڶ�̬�����Ƿ���ͶӰ�ǽ���/�γ���TIMER
#define CAM_SHOW_PIC_L			4			//����չʾ�����ͼ��
#define CAM_SHOW_PIC_R			5			//����չʾ�����ͼ��
#define MAX_CAMERA_NUM 20

// CBinocularDlg �Ի���
class CBinocularDlg : public CDialogEx
{
	// ����
public:
	CBinocularDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CBinocularDlg();
// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BINOCULAR_V_1_DIALOG };
#endif
	
	//һЩ��Ҫ�ı���
	Camera* left_Camera;
	Camera* right_Camera;
	Projector* projector;
	unsigned long cam_online_num;
	std::string log_buffer;
	unsigned long frmcnt_buffer_l;
	unsigned long frmcnt_buffer_r;

	//���ڼ�����λ����άЧ���Ĳ��� ���д洢��ԭʼͼƬ 
	//��֤�ڴ���Ĺ����У����е�ͼ��ready��ǩ�������µ� �����ǲ��������µ�ͼ�����˴���
	volatile bool left_need_unwrap;
	bool left_is_unwrap;
	cv::Mat* buffer_left;
	bool* left_buffer_ready;
	CRITICAL_SECTION* left_buffer_cs;	//�ڽ��������������Ӷ����д����ʱ��������Ҫ��ʱ�ĸ��������ͼ��	
	CWinThread *left_unwrap_h;

	volatile bool right_need_unwrap;
	bool right_is_unwrap;
	cv::Mat* buffer_right;
	bool* right_buffer_ready;
	CRITICAL_SECTION* right_buffer_cs;	//�ڽ��������������Ӷ����д����ʱ��������Ҫ��ʱ�ĸ��������ͼ��
	CWinThread *right_unwrap_h;

	//������GPU���д����ʱ������Ҳ���Խ�ͼƬֱ�ӵ���GPU�б������Ŀ��������ƹ����ȡ�

	//���ڽ���λ�Ĳ�����Ϣ
	Proj_Strategy strtgy;
	std::vector<cv::Mat> left_warp_phase;		//�����������λ	������Ҫ���£�
	std::vector<cv::Mat> right_warp_phase;		//�����������λ	������Ҫ���£�
	cv::Mat left_unwarp_phase;					//������������λ	��ʱ�̸��£�
	cv::Mat right_unwarp_phase;					//������������λ  ��ʱ�̸��£�
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	//����ʱ�� ���������
	afx_msg void OnTimer(UINT_PTR uId);

	DECLARE_MESSAGE_MAP()
public:
	DWORD _left_ip;
	DWORD _right_ip;

	CComboBox _triger_type;
	CEdit _log_ctrl;

	CStatic _right_frate;
	CStatic _left_frate;

	CEdit _left_expose_value;
	CEdit _right_expose_value;
	CEdit _frame_set_l;
	CEdit _frame_set_r;
	
	CMFCEditBrowseCtrl _dir_path;
	CMFCEditBrowseCtrl _ini_path;

	CButton _btn_connectproj;
	CButton _btn_closeproj;

	CEdit _edt_proj_exp;
	CEdit _edt_proj_prd;

	CEdit _cycle_1;
	CEdit _cycle_2;
	CEdit _cycle_3;
	CEdit _step_1;
	CEdit _step_2;
	CEdit _step_3;

	CButton _is_saving;
	CButton _is_repeat;

	CButton _btn_startexp;
	CButton _btn_unwarp;
	CButton _btn_calc3d;
	CButton _btn_stopacq;

	afx_msg void OnSelchangeCmbTrigger();
	afx_msg void OnBnClickedBtnopenleft();
	afx_msg void OnBnClickedBtncloseleft();
	afx_msg void OnBnClickedBtnopenright();
	afx_msg void OnBnClickedBtncloseright();
	afx_msg void OnBnClickedBtnStartacq();
	afx_msg void OnBnClickedBtnStopacq();
	afx_msg void OnBnClickedBtnSetexposel();
	afx_msg void OnBnClickedBtnSetexposer();
	afx_msg void OnBnClickedChkIssaving();
	afx_msg void OnEnChangeMfceditbrowse1();
	afx_msg void OnBnClickedBtnStartexp();
	afx_msg void OnEnChangeMfceditbrowse2();
	afx_msg void OnBnClickedBtnCloseproj();
	afx_msg void OnBnClickedBtnConnectproj();
	afx_msg void OnEnChangeEdtExposeurproj();
	afx_msg void OnEnChangeEdtPeriodproj();
	afx_msg void OnEnChangeEdtCycle1();
	afx_msg void OnEnChangeEdtCycle2();
	afx_msg void OnEnChangeEdtCycle3();
	afx_msg void OnEnChangeEdtStep1();
	afx_msg void OnEnChangeEdtStep2();
	afx_msg void OnEnChangeEdtStep3();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedBtnEnd();
	afx_msg void OnBnClickedBtnUnwarp();

	CRITICAL_SECTION Log_Protection;		//��־���̲߳�����Ҫ�ı���
	CRITICAL_SECTION Proj_Protection;		//���ڱ������ ֻ�е����߳��ܹ����������USB��д����

	//�������õĺ��� ���������������
	void append_log(std::string& log_data);		//��Log�������
	void update_projector_status();				//����ͶӰ�ǵ�״̬��Ϣ
	void update_show(Camera * cam, UINT ID);
};

//����� ���߳̿�ʼ�ɼ����� ������Ϊ�˱���UI���濨�� ���Կ���һ�����߳�ȥ����
UINT Left_ThreadCapture(LPVOID lpParam);		
UINT Right_ThreadCapture(LPVOID lpParam);

//����� ���̰߳�����λ���㺯��	�����Ĺ������߳� ���ڼ������ݣ��������ݴ������ �ڻ�ȡͼ��Ĺ����У�һ����ȡͼ���жϣ���ô����ֹͣ����
UINT Left_ThreadUnwarp(LPVOID lpParam);
UINT Right_ThreadUnwarp(LPVOID lpParam);
