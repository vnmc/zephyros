/*******************************************************************************
 * Copyright (c) 2015-2016 Vanamco AG, http://www.vanamco.com
 *
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 *******************************************************************************/


#include <algorithm>
#include <tchar.h>

#include "base/types.h"
#include "components/dialog_win.h"


#define BUF_SIZE 4096


extern HINSTANCE g_hInst;


BYTE* align(BYTE* lpIn, INT_PTR dw2Power = 4)
{
    INT_PTR ul = (INT_PTR) lpIn;
    ul += dw2Power - 1;
    ul &= ~(dw2Power - 1);

    return (BYTE*) ul;
}


Dialog::Dialog(WORD wDlgResId, HWND hwndParent)
  : m_hwnd(NULL),
    m_hwndParent(hwndParent),
    m_wDlgResId(wDlgResId),
    m_pBuf(NULL)
{
}

Dialog::Dialog(String strCaption, HWND hwndParent, int nWidth, int nHeight)
  : m_hwnd(NULL),
    m_hwndParent(hwndParent),
    m_wDlgResId(0),
    m_pBuf(NULL),
    m_nWidth(nWidth),
    m_nHeight(nHeight)
{
    m_pBuf = new BYTE[BUF_SIZE];
    ZeroMemory(m_pBuf, BUF_SIZE);

    m_ptr = align(m_pBuf, 2);

    DWORD dwExStyle = 0;
    DWORD dwStyle = DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION | WS_SYSMENU;

    WriteData<WORD>(1);         // dlgVer
    WriteData<WORD>(0xffff);    // signature
    WriteData<DWORD>(0);        // help ID
    WriteData<DWORD>(dwExStyle);// exstyle
    WriteData<DWORD>(dwStyle);  // style

    m_pDlgItemsCount = (WORD*) m_ptr;
    WriteData<WORD>(0);         // cDlgItems
    
    WriteData<short>(0);        // x
    WriteData<short>(0);        // y
    
    m_pWidth = (short*) m_ptr;
    WriteData<short>(0);        // cx
    m_pHeight = (short*) m_ptr;
    WriteData<short>(0);        // cy
    
    WriteData<WORD>(0x0000);    // menu
    WriteData<WORD>(0x0000);    // windowClass
    WriteUnicodeString(strCaption.c_str());

    if (dwStyle & (DS_SETFONT | DS_SHELLFONT))
    {
        WriteData<WORD>(8);         // point size
        WriteData<WORD>(FW_NORMAL); // weight
        WriteData<BYTE>(FALSE);     // italic
        WriteData<BYTE>(DEFAULT_CHARSET);
        WriteUnicodeString(TEXT("MS Shell Dlg"));
    }
}

Dialog::~Dialog()
{
    if (m_pBuf)
        delete[] m_pBuf;
}

void Dialog::AddControl(const TCHAR* szCaption, int nResourceID, int nCtrlClass, const TCHAR* szWindowClass, int nID, int x, int y, int nWidth, int nHeight, int nStyle, int nExStyle)
{
    // align DLGITEMTEMPLATE on DWORD boundary
    m_ptr = align(m_ptr);

    WriteData<DWORD>(0);    // help ID
    WriteData<DWORD>(nExStyle);
    WriteData<DWORD>(nStyle);
    WriteData<short>(x);
    WriteData<short>(y);
    WriteData<short>(nWidth);
    WriteData<short>(nHeight);
    WriteData<DWORD>(nID);

    // window class
    if (szWindowClass)
        WriteUnicodeString(szWindowClass);
    else
    {
        WriteData<WORD>(0xffff);
        WriteData<WORD>(nCtrlClass);
    }

    // caption
    if (szCaption)
        WriteUnicodeString(szCaption);
    else
    {
        WriteData<WORD>(0xffff);
        WriteData<WORD>(nResourceID);
    }

    WriteData<WORD>(0); // extra count (creation data)

    // modify the dialog template (update the number of controls and the geometry)
    (*m_pDlgItemsCount)++;
    if (m_nWidth == 0)
        *m_pWidth = std::max(*m_pWidth, (short) (x + nWidth + 16));
    if (m_nHeight == 0)
        *m_pHeight = std::max(*m_pHeight, (short) (y + nHeight + 16));
}

void Dialog::WriteUnicodeString(const TCHAR* szString)
{
#ifdef _UNICODE
    _tcscpy((TCHAR*) m_ptr, szString);
    m_ptr += (_tcslen(szString) + 1) * sizeof(TCHAR);
#else
    m_ptr += MultiByteToWideChar(CP_ACP, 0, szString, -1, (LPWSTR) m_ptr, BUF_SIZE - ((INT_PTR) m_ptr - (INT_PTR) m_pTemplate)) * sizeof(TCHAR);
#endif
}

template<typename T> void Dialog::WriteData(T datum)
{
    *(reinterpret_cast<T*>(m_ptr)) = datum;
    m_ptr += sizeof(T);
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
        (LPDLGTEMPLATE) align(m_pBuf, 2),
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
