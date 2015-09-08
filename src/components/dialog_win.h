/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
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

#ifndef Zephyros_DialogWin_h
#define Zephyros_DialogWin_h
#pragma once


#include <Windows.h>
#include <map>


#define ON_MESSAGE(msg, func) m_mapMessages[msg] = reinterpret_cast<MessageHandler>(&func);
#define ON_COMMAND(cmd, func) m_mapCommands[cmd] = reinterpret_cast<CommandHandler>(&func);

#define ADD_CONTROL(type, enforce_style) { AddControl(strCaption, type, nID, x, y, nWidth, nHeight, nStyle | enforce_style, nExStyle); }


class Dialog;

typedef void (Dialog::*MessageHandler)(WPARAM, LPARAM);
typedef void (Dialog::*CommandHandler)(WORD, LPARAM);


class Dialog
{
public:
	Dialog(WORD wDlgResId, HWND hwndParent = NULL);
	Dialog(String strCaption, HWND hwndParent = NULL, int nWidth = 0, int nHeight = 0);
	~Dialog();

	void AddControl(const TCHAR* szCaption, int nResourceID, int nCtrlType, const TCHAR* szWindowClass, int nID, int x, int y, int nWidth, int nHeight, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0);
	inline void AddControl(String strCaption, int nCtrlType, int nID, int x, int y, int nWidth, int nHeight, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) { AddControl(strCaption.c_str(), -1, nCtrlType, NULL, nID, x, y, nWidth, nHeight, nStyle, nExStyle); }
	inline void AddControl(String strCaption, const TCHAR* szWindowClass, int nID, int x, int y, int nWidth, int nHeight, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) { AddControl(strCaption.c_str(), -1, 0, szWindowClass, nID, x, y, nWidth, nHeight, nStyle, nExStyle); }
	
	inline void AddStatic(String strCaption, int x, int y, int nWidth, int nHeight, int nID = -1, int nStyle = WS_CHILD | WS_VISIBLE | SS_LEFT, int nExStyle = 0) ADD_CONTROL(0x0082, 0)
	inline void AddIcon(int nIconID, int x, int y, int nWidth, int nHeight, int nID = -1, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) { AddControl(NULL, nIconID, 0x0082, NULL, nID, x, y, nWidth, nHeight, nStyle, nExStyle); }
	inline void AddGroupBox(String strCaption, int x, int y, int nWidth, int nHeight, int nID = -1, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0080, BS_GROUPBOX)
	inline void AddButton(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0080, BS_PUSHBUTTON)
	inline void AddDefaultButton(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0080, BS_DEFPUSHBUTTON)
	inline void AddCheckBox(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0080, BS_AUTOCHECKBOX)
	inline void AddRadioButton(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0080, BS_AUTORADIOBUTTON)
	inline void AddTextBox(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0081, 0)
	inline void AddComboBox(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x083, LBS_COMBOBOX)
	inline void AddListBox(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0083, 0)
	inline void AddVScrollBar(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0083, SBS_VERT)
	inline void AddHScrollBar(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(0x0083, SBS_HORZ)
	inline void AddProgressBar(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(TEXT("msctls_progress32"), 0)
	inline void AddTrackBar(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(TEXT("msctls_trackbar32"), 0)
	inline void AddTab(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(TEXT("SysTabControl32"), 0)
	inline void AddListView(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(TEXT("SysListView32"), 0)
	inline void AddTreeView(String strCaption, int x, int y, int nWidth, int nHeight, int nID, int nStyle = WS_CHILD | WS_VISIBLE, int nExStyle = 0) ADD_CONTROL(TEXT("SysTreeView32"), 0)

	INT_PTR DoModal();

private:
	static INT_PTR CALLBACK MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void WriteUnicodeString(const TCHAR* szString);
	template<typename T> void WriteData(T datum);

protected:
	std::map<UINT, MessageHandler> m_mapMessages;
	std::map<WORD, CommandHandler> m_mapCommands;

	HWND m_hwnd;
	HWND m_hwndParent;
	WORD m_wDlgResId;
	//HGLOBAL m_hglTemplate;
	BYTE* m_pBuf;
	//LPDLGTEMPLATE m_pTemplate;
	int m_nWidth;
	int m_nHeight;
	WORD* m_pDlgItemsCount;
	short* m_pWidth;
	short* m_pHeight;
	BYTE* m_ptr;
};


#endif // Zephyros_DialogWin_h
