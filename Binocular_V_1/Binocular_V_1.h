
// Binocular_V_1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CBinocularApp: 
// �йش����ʵ�֣������ Binocular_V_1.cpp
//

class CBinocularApp : public CWinApp
{
public:
	CBinocularApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CBinocularApp theApp;