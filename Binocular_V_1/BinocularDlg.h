
// BinocularDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
//�Զ���� �����
#include "Camera.h"
//�Զ���� ͶӰ����
#include "Projector.h"

#include <string>
#include "afxeditbrowsectrl.h"

#define CAM_FINDER_TIMER_ID		1			//���ڶ�̬�����Ƿ����������/�γ���TIMER
#define CAM_FRAMERATE_GET_ID    2			//���ڶ�̬������ͼ��������TIMER
#define PROJ_FINDER_TIMER_ID	3			//���ڶ�̬�����Ƿ���ͶӰ�ǽ���/�γ���TIMER
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
	CWinThread* left_thread;
	CWinThread* right_thread;
	unsigned long frmcnt_buffer_l;
	unsigned long frmcnt_buffer_r;

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
	CString _setted_exp_val;
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
	CButton _is_saving;
	CMFCEditBrowseCtrl _dir_path;
	CMFCEditBrowseCtrl _ini_path;

	CButton _btn_connectproj;
	CButton _btn_closeproj;

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

	CRITICAL_SECTION Log_Protection;		//��־���̲߳�����Ҫ�ı���

	//�������õĺ��� ���������������
	void append_log(std::string& log_data);		//��Log�������
	void update_projector_status();				//����ͶӰ�ǵ�״̬��Ϣ
};

//����� ���̲߳ɼ�����
UINT Left_ThreadCapture(LPVOID lpParam);
UINT Right_ThreadCapture(LPVOID lpParam);