#pragma once


#include <Windows.h>
#include <map>


#define ON_MESSAGE(msg, func) m_mapMessages[msg] = reinterpret_cast<MessageHandler>(&func);
#define ON_COMMAND(cmd, func) m_mapCommands[cmd] = reinterpret_cast<CommandHandler>(&func);


class Dialog;

typedef void (Dialog::*MessageHandler)(WPARAM, LPARAM);
typedef void (Dialog::*CommandHandler)(WORD, LPARAM);


class Dialog
{
public:
	Dialog(WORD wDlgResId, HWND hwndParent = NULL);
	INT_PTR DoModal();

private:
	static INT_PTR CALLBACK MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	std::map<UINT, MessageHandler> m_mapMessages;
	std::map<WORD, CommandHandler> m_mapCommands;

	HWND m_hwnd;
	HWND m_hwndParent;
	WORD m_wDlgResId;
};