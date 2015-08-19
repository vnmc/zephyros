#include "dialog_win.h"

#define BUF_SIZE 1024


extern HINSTANCE g_hInst;


LPWORD align(LPWORD lpIn, ULONG dw2Power = 4)
{
    ULONG ul;

    ul = (ULONG) lpIn;
    ul += dw2Power - 1;
    ul &= ~(dw2Power - 1);

    return (LPWORD) ul;
}


Dialog::Dialog(WORD wDlgResId, HWND hwndParent)
  : m_hwnd(NULL),
	m_hwndParent(hwndParent),
	m_wDlgResId(wDlgResId),
	m_hglTemplate(NULL),
	m_pTemplate(NULL)
{
}

Dialog::Dialog(String strCaption, HWND hwndParent, int nWidth, int nHeight)
  : m_hwnd(NULL),
	m_hwndParent(hwndParent),
	m_wDlgResId(0),
	m_hglTemplate(NULL),
	m_pTemplate(NULL),
	m_nWidth(nWidth),
	m_nHeight(nHeight)
{
	m_hglTemplate = GlobalAlloc(GMEM_ZEROINIT, BUF_SIZE);
	if (m_hglTemplate)
		m_pTemplate = (LPDLGTEMPLATE) GlobalLock(m_hglTemplate);

	m_pTemplate->style = DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;

	// number of controls
	m_pTemplate->cdit = 0;

	// dialog geometry
    m_pTemplate->x = 0;
    m_pTemplate->y  = 0;
    m_pTemplate->cx = m_nWidth;
    m_pTemplate->cy = m_nHeight;

    m_ptr = (LPWORD) (lpdt + 1);
    *m_ptr++ = 0; // no menu
    *m_ptr++ = 0; // predefined dialog box class (by default)

    WriteUnicodeString(strCaption.c_str());
}

Dialog::~Dialog()
{
	if (m_pTemplate)
		GlobalUnlock(m_hglTemplate);
	if (m_hglTemplate != NULL)
		GlobalFree(m_hglTemplate);
}

void Dialog::AddControl(const TCHAR* szCaption, int nResourceID, int nCtrlClass, const TCHAR* szWindowClass, int nID, int x, int y, int nWidth, int nHeight, int nStyle)
{
	// align DLGITEMTEMPLATE on DWORD boundary
	m_ptr = align(m_ptr);

    LPDLGITEMTEMPLATE pItemTpl = (LPDLGITEMTEMPLATE) m_ptr;

    pItemTpl->x = x;
    pItemTpl->y = y;
    pItemTpl->cx = nWidth;
    pItemTpl->cy = nHeight;

    lpdit->id = nID;
    lpdit->style = nStyle;

    m_ptr = (LPWORD) (pItemTpl + 1);

    // window class
    if (szWindowClass)
    	WriteUnicodeString(szWindowClass);
    else
    {
    	*m_ptr++ = 0xffff;
    	*m_ptr++ = nCtrlClass;
    }    	

    // caption
    if (szCaption)
	    WriteUnicodeString(szCaption);
	else
    {
    	*m_ptr++ = 0xffff;
    	*m_ptr++ = nResourceID;
    }

    // no creation data
    *m_ptr++ = 0;

    // modify the dialog template (update the number of controls and the geometry)
    m_pTemplate->cdit++;
    if (m_nWidth == 0)
    	m_pTemplate->cx = std::max(m_pTemplate->cx, x + nWidth + 16);
    if (m_nHeight == 0)
    	m_pTemplate->cy = std::max(m_pTemplate->cy, y + nHeight + 16);
}

void Dialog::WriteUnicodeString(TCHAR* szString)
{
#ifdef _UNICODE
	_tcscpy((TCHAR*) m_ptr, szString);
	m_ptr += (_tcslen(szString) + 1) * sizeof(TCHAR);
#else
    m_ptr += MultiByteToWideChar(CP_ACP, 0, szString, -1, (LPWSTR) m_ptr, BUF_SIZE - ((PTR) m_ptr - (PTR) m_pTemplate));
#endif
}

INT_PTR Dialog::DoModal()
{
	// show the dialog box
	if (m_wDlgResId != 0)
	{
		return DialogBoxParam(
			g_hInst,
			MAKEINTRESOURCE(m_wDlgResId),
			m_hwndParent,
			Dialog::MsgProc,
			reinterpret_cast<LPARAM>(this)
		);
	}

	return DialogBoxIndirectParam(
		g_hInst, 
        (LPDLGTEMPLATE) m_hglTemplate,
        m_hwndParent,
        Dialog::MsgProc,
        reinterpret_cast<LPARAM>(this)
	);
}

INT_PTR CALLBACK Dialog::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
	{
		// in "init dialog," set a pointer to "this" in the window's user data and save the window handle
		SetWindowLongPtr(hWnd, GWLP_USERDATA, static_cast<LONG_PTR>(lParam));
		(reinterpret_cast<Dialog*>(lParam))->m_hwnd = hWnd;
	}

	// try to get the dialog instance
	Dialog* pDlg = reinterpret_cast<Dialog*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pDlg == NULL)
		return FALSE;

	if (message == WM_COMMAND)
	{
		// handle command messages
		std::map<WORD, CommandHandler>::iterator it = pDlg->m_mapCommands.find(LOWORD(wParam));
		if (it != pDlg->m_mapCommands.end())
		{
			(pDlg->*(it->second))(HIWORD(wParam), lParam);
			return TRUE;
		}
	}
	else
	{
		// handle other messages
		std::map<UINT, MessageHandler>::iterator it = pDlg->m_mapMessages.find(message);
		if (it != pDlg->m_mapMessages.end())
		{
			(pDlg->*(it->second))(wParam, lParam);
			return TRUE;
		}
	}

	return FALSE;
}
