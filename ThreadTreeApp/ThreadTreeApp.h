
// ThreadTreeApp.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "DbgHelp.lib")

#ifndef __AFXWIN_H__
	#error "включить stdafx.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CThreadTreeApp:
// Сведения о реализации этого класса: ThreadTreeApp.cpp
//

class CThreadTreeApp : public CWinApp
{
public:
	CThreadTreeApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CThreadTreeApp theApp;
