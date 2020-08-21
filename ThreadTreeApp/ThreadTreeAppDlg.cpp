
// ThreadTreeAppDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "ThreadTreeApp.h"
#include "ThreadTreeAppDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Диалоговое окно CAboutDlg используется для описания сведений о приложении

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

// Реализация
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Диалоговое окно CThreadTreeAppDlg



CThreadTreeAppDlg::CThreadTreeAppDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_THREADTREEAPP_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CThreadTreeAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CThreadTreeAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CThreadTreeAppDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_SAVE, &CThreadTreeAppDlg::OnBnClickedSave)
END_MESSAGE_MAP()


// Обработчики сообщений CThreadTreeAppDlg

BOOL CThreadTreeAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->SetWindowTextW(L"ThreadTreeApp");
	// Добавление пункта "О программе..." в системное меню.

	// IDM_ABOUTBOX должен быть в пределах системной команды.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CThreadTreeAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CThreadTreeAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CThreadTreeAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CThreadTreeAppDlg::OnBnClickedStart()
{
	CEdit* pPIDEdit = static_cast<CEdit*>(this->GetDlgItem(IDC_PID_EDIT));
	wchar_t szPID[16];
	*((WORD*)&szPID[0]) = 16;
	pPIDEdit->GetLine(0, szPID);
	DWORD pid = wcstol(szPID, nullptr, 10);

	try
	{
		threadTree.createTree(pid);
	}
	catch (std::exception& e)
	{
		MessageBoxA(this->GetSafeHwnd(), e.what(), "Error", MB_ICONERROR);
		return;
	}

	CTreeCtrl* pTree = static_cast<CTreeCtrl*>(this->GetDlgItem(IDC_TREE1));
	pTree->DeleteAllItems();
	
	for (auto& t : threadTree.getRoot())
	{
		HTREEITEM thread = pTree->InsertItem(L"Thread");
		pTree->InsertItem((std::wstring(L"Id: ")+std::to_wstring(t.id)).c_str(), thread);
		pTree->InsertItem((std::wstring(L"State: ") + (t.state == ThreadNode::Running ? L"Running" : L"Finished")).c_str(), thread);
		pTree->Expand(thread, TVE_EXPAND);

		HTREEITEM callstack = pTree->InsertItem(L"Callstack", thread);
		for (auto& s : t.stacktrace)
		{
			pTree->InsertItem((std::wstring(L"Module name: ") + s.moduleName).c_str(), callstack);
			pTree->Expand(callstack, TVE_EXPAND);
			callstack = pTree->InsertItem((std::wstring(L"Function name: ") + s.functionName).c_str(), callstack);
		}
	}
}


void CThreadTreeAppDlg::OnBnClickedSave()
{
	CFileDialog saveFileDialog(FALSE, NULL, L"ThreadTree.xml", 6, L"File XML (*.xml)|*.xml|");
	int result = saveFileDialog.DoModal();
	if (result == IDOK)
	{
		CFile saveFile(saveFileDialog.GetPathName(), CFile::modeWrite | CFile::modeCreate);
		std::wstring buf = threadTree.toXML();
		saveFile.Write(buf.c_str(), buf.size()*sizeof(wchar_t));
		saveFile.Flush();
	}
}
