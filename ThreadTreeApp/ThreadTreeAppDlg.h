
// ThreadTreeAppDlg.h: файл заголовка
//

#pragma once

#include "ThreadTree.h"

// Диалоговое окно CThreadTreeAppDlg
class CThreadTreeAppDlg : public CDialogEx
{
// Создание
public:
	CThreadTreeAppDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THREADTREEAPP_DIALOG };
#endif
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;
	ThreadTree threadTree;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedSave();
};
