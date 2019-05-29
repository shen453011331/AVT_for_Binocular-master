
// BinocularDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
//自定义的 相机类
#include "Camera.h"
//自定义的 投影仪类
#include "Projector.h"

#include <string>
#include "afxeditbrowsectrl.h"

#define CAM_FINDER_TIMER_ID		1			//用于动态发现是否有相机接入/拔出的TIMER
#define CAM_FRAMERATE_GET_ID    2			//用于动态获得相机图像张数的TIMER
#define PROJ_FINDER_TIMER_ID	3			//用于动态发现是否有投影仪接入/拔出的TIMER
#define MAX_CAMERA_NUM 20
// CBinocularDlg 对话框
class CBinocularDlg : public CDialogEx
{
	// 构造
public:
	CBinocularDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CBinocularDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BINOCULAR_V_1_DIALOG };
#endif
	
	//一些重要的变量
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
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	//各种时钟 添加在这里
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

	CRITICAL_SECTION Log_Protection;		//日志单线程操作需要的保护

	//各种有用的函数 今后可以添加在这里
	void append_log(std::string& log_data);		//对Log进行添加
	void update_projector_status();				//更新投影仪的状态信息
};

//单相机 单线程采集函数
UINT Left_ThreadCapture(LPVOID lpParam);
UINT Right_ThreadCapture(LPVOID lpParam);