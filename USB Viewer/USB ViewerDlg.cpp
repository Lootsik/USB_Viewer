
// USB ViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "USB Viewer.h"
#include "USB ViewerDlg.h"
#include "afxdialogex.h"

#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
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


// CUSBViewerDlg dialog



CUSBViewerDlg::CUSBViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_USBVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//LoadMenu(MenuTrayIcon);

}


CUSBViewerDlg::~CUSBViewerDlg()
{
	
}


void CUSBViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, ID_LIST_BOX, ListBoxCtrl);
}

BEGIN_MESSAGE_MAP(CUSBViewerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//device list changed
	ON_WM_DEVICECHANGE()
	//some event with tray icon
	ON_MESSAGE(WM_ShellNote,&CUSBViewerDlg::OnTrayEvent)
	ON_COMMAND(WM_ShowWindow,&CUSBViewerDlg::ShowMainWindow)

	ON_MESSAGE(WM_DESTROY, &CUSBViewerDlg::DestroyTrayIncon)
	ON_COMMAND(IDMenuUpdate, &CUSBViewerDlg::OnMenuUpdate)
	ON_COMMAND(IDMenuExit, &CUSBViewerDlg::OnMenuExit)
	ON_COMMAND(ID_ABOUT_INFO, &CUSBViewerDlg::OnAboutInfo)
	ON_LBN_SELCHANGE(ID_LIST_BOX, &CUSBViewerDlg::OnLbnSelchangeList)
	ON_BN_CLICKED(ID_BUTTON_CHANGE_STATE, &CUSBViewerDlg::OnBnClickedChangeState)
END_MESSAGE_MAP()


LRESULT CUSBViewerDlg::DestroyTrayIncon(WPARAM wParam, LPARAM lParam)
{
	DestroyIcon(nid.hIcon);
	BOOL bSuccess = Shell_NotifyIcon(NIM_DELETE, &nid);
	CDialog::OnOK();
	return 0;
}



BOOL CUSBViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CMenu WindowMenu;
	WindowMenu.LoadMenuW(MenuTrayIcon);
	SetMenu(&WindowMenu);

	//устанавливаем иконку в трей
	isTrayIconSet = CreateTrayIcon();

	//create info about USB devices
	SilentUpdateDevices();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CUSBViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	//fold up
	if (nID == SC_MINIMIZE && isTrayIconSet)
	{
		FoldUpToTray();
	}

	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}


void CUSBViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CUSBViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CUSBViewerDlg::CreateTrayIcon()
{
	memset(&nid, 0, sizeof(nid));
	nid.cbSize = sizeof(nid);

	nid.hWnd = m_hWnd;
	nid.uID = ID_TrayIcon;
	nid.uCallbackMessage = WM_ShellNote;

	HICON MicroIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDR_MAINFRAME),
		IMAGE_ICON,
		16, 16, // use actual size
		LR_DEFAULTCOLOR
	);
	nid.hIcon = MicroIcon;

	lstrcpyn(nid.szTip, L"USB Viewer running", sizeof(nid.szTip) / sizeof(nid.szTip[0]));
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;

	return Shell_NotifyIcon(NIM_ADD, &nid);
}

//tray icon event handler
LRESULT CUSBViewerDlg::OnTrayEvent(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)lParam)
	{

	//lbm opens window
	case WM_LBUTTONUP:
	{
		ShowMainWindow();
	}break;

	//rb opens dialog
	case WM_RBUTTONUP:
	{
		HMENU hMenu = ::LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(MenuTrayIcon));
		if (!hMenu)
			return FALSE;
		HMENU hPopup = ::GetSubMenu(hMenu, 0);
		if (!hPopup)
			return FALSE;

		SetForegroundWindow();

		POINT pt;
		GetCursorPos(&pt);
		TrackPopupMenu(hPopup, 0, pt.x, pt.y, 0, GetSafeHwnd(), NULL);

		DestroyMenu(hMenu);
	}break;

	}
	return 0;
}


//device change event handler
int CUSBViewerDlg::OnDeviceChange(UINT nEventType, DWORD_PTR dwData)
{
	//1 second
	UINT Timeout = 1000;
	const UINT dT = 200;

	if (!USB::UpdateInfo(USBDevices))
	{
		СrashShutdown();
	}
	//release unchecked
	for (int i = USBDevices.size() - 1; i >= 0; --i)
	{
		if (USBDevices[i].EntState == USB::EntryState::UNCHECKED)
		{
			SendNotification(L"USB device detached", USBDevices[i].DeviceInfoString(), Timeout);
			Timeout += dT;
			USBDevices.erase(USBDevices.begin() + i);
		}
	}

	//Alert new and mark as unchecked
	size_t size = USBDevices.size();
	for (size_t i = 0;i < size ;++i)
	{
		if (USBDevices[i].EntState == USB::EntryState::NEW)
		{
			SendNotification(L"New USB device", USBDevices[i].DeviceInfoString(), Timeout);
			Timeout += dT;
		}
		else if (USBDevices[i].EntState == USB::EntryState::CHANGED_STATE)
		{
			if (USBDevices[i].DevState == USB::DeviceState::ENABLED)
				SendNotification(L"USB device enabled", USBDevices[i].DeviceInfoString(), Timeout);
			else if (USBDevices[i].DevState == USB::DeviceState::DISABLED)
				SendNotification(L"USB device disabled", USBDevices[i].DeviceInfoString(), Timeout);

			Timeout += dT;
		}
		USBDevices[i].EntState = USB::EntryState::UNCHECKED;
	}
	UpdateDeviceView();
	return 0;
}


void CUSBViewerDlg::SendNotification(const TCHAR* Title, const TCHAR* Text, UINT Timeout)
{
	nid.hWnd = m_hWnd;
	lstrcpy(nid.szInfoTitle, Title);
	lstrcpy(nid.szInfo, Text);

	nid.uTimeout = Timeout;
	nid.uFlags = NIF_INFO | NIF_TIP;

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void CUSBViewerDlg::ShowMainWindow()
{
	ShowWindow(SW_SHOW);
	ShowWindow(SW_RESTORE);
	this->SetActiveWindow();
}

void CUSBViewerDlg::FoldUpToTray()
{
	ShowWindow(SW_MINIMIZE);
	ShowWindow(SW_HIDE);
}

void CUSBViewerDlg::СrashShutdown()
{
	MessageBox(L"Cannot get informations about devices", L"Error");
	CDialog::OnCancel();
}

//update devices without notifications
void CUSBViewerDlg::SilentUpdateDevices()
{
	if (!USB::UpdateInfo(USBDevices))
	{
		СrashShutdown();
	}
	for (auto& Item : USBDevices)
		Item.EntState = USB::EntryState::UNCHECKED;
	//refresh list in window
	UpdateDeviceView();
}

void CUSBViewerDlg::UpdateDeviceView()
{
	//clear info
	SetDlgItemText(ID_INFO_EDIT,L"");
	//hide button
	GetDlgItem(ID_BUTTON_CHANGE_STATE)->ShowWindow(SW_HIDE);
	//delete all items
	ListBoxCtrl.ResetContent();

	size_t size = USBDevices.size();
	for (size_t i = 0; i < size; ++i)
	{
		ListBoxCtrl.InsertString(i, USBDevices[i].Description);
	}
}

void CUSBViewerDlg::OnMenuUpdate()
{
	USBDevices.clear();
	SilentUpdateDevices();
}

void CUSBViewerDlg::OnMenuExit()
{
	SendMessage(WM_DESTROY);
}

void CUSBViewerDlg::OnAboutInfo()
{
	CAboutDlg dlg;
	dlg.DoModal();
}

void CUSBViewerDlg::OnLbnSelchangeList()
{
	//get selected item
	UINT i = ListBoxCtrl.GetCurSel();

	if (i >= USBDevices.size())
		return;

	CString TmpStr;
	CString InfoStr;
	
	std::wstringstream Stream;

	const wchar_t* end = L"\r\n";

	Stream << L"InstanceID: " << USBDevices[i].InstanceID.GetString() << end;

	if (USBDevices[i].DevState == USB::DeviceState::ENABLED)
		Stream << L"State: ENABLED" << end;
	else if (USBDevices[i].DevState == USB::DeviceState::DISABLED)
		Stream << L"State: DISABLED" << end;

	if ( !USBDevices[i].Description.IsEmpty())
		Stream << L"Description: " << USBDevices[i].Description.GetString() << end;

	if ( !USBDevices[i].BusReportedDeviceDescription.IsEmpty()) 
		Stream << L"Bus Reported Device Description: " << USBDevices[i].BusReportedDeviceDescription.GetString() << end;

	if ( !USBDevices[i].Manufacturer.IsEmpty())
		Stream << L"Manufacturer: " << USBDevices[i].Manufacturer.GetString() << end;

	if ( !USBDevices[i].FriendlyName.IsEmpty() )
		Stream << L"Friendly Name: " << USBDevices[i].FriendlyName.GetString() << end;

	if ( !USBDevices[i].LocationInfo.IsEmpty())
		Stream << L"Location Info: " << USBDevices[i].LocationInfo.GetString() << end;

	if ( !USBDevices[i].Vid.IsEmpty()) 
		Stream << end << L"Vid: " << USBDevices[i].Vid.GetString() << end;
	
	if ( !USBDevices[i].Pid.IsEmpty() ) 
		Stream << L"Pid: " << USBDevices[i].Pid.GetString() << end;
	
	if (!USBDevices[i].Mi.IsEmpty())
		Stream << L"Mi: " << USBDevices[i].Mi.GetString() << end;


	SetDlgItemText(ID_INFO_EDIT, Stream.str().c_str());

	//set label and show button
	if (USBDevices[i].DevState == USB::DeviceState::ENABLED)
	{
		SetDlgItemText(ID_BUTTON_CHANGE_STATE, L"Disable device");
		GetDlgItem(ID_BUTTON_CHANGE_STATE)->ShowWindow(SW_SHOW);

	}
	else if (USBDevices[i].DevState == USB::DeviceState::DISABLED)
	{
		SetDlgItemText(ID_BUTTON_CHANGE_STATE, L"Enable device");
		GetDlgItem(ID_BUTTON_CHANGE_STATE)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(ID_BUTTON_CHANGE_STATE)->ShowWindow(SW_HIDE);
	}
}


void CUSBViewerDlg::OnBnClickedChangeState()
{
	UINT i = ListBoxCtrl.GetCurSel();
	//0xffffffff - noone changed
	if (i >= USBDevices.size())
		return;

	USB::DeviceState DevState;

	if (USBDevices[i].DevState == USB::DeviceState::ENABLED )
		DevState = USB::DeviceState::DISABLED;
	else
		DevState = USB::DeviceState::ENABLED;

	switch (USB::ChangeDevState(USBDevices[i], DevState))
	{
	//Ok
	case USB::ChangeStateResult::OK:
		break;
	//access denied
	case USB::ChangeStateResult::NEED_ADMIN:

		MessageBox(L"You must run this application with administrator privilege to change this parameter", L"Access denied", MB_ICONERROR);
		break;
	
	case USB::ChangeStateResult::NOT_DISABLEABLE:
		MessageBox(L"This device cannnot be disabled", L"Error", MB_ICONERROR);
		break;
	
	default:
		MessageBox(L"This parameter cannot be changed", L"Error", MB_ICONERROR);
	}
}
