#include "dialog_win.h"


extern HINSTANCE g_hInst;


Dialog::Dialog(WORD wDlgResId, HWND hwndParent)
	: m_hwnd(NULL), m_hwndParent(hwndParent), m_wDlgResId(wDlgResId)
{
}

INT_PTR Dialog::DoModal()
{
	// show the dialog box
	return DialogBoxParam(
		g_hInst,
		MAKEINTRESOURCE(m_wDlgResId),
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
