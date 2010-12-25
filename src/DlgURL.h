#pragma once
#include <string>
#include <windows.h>
#include <aygshell.h>

class DialogURL
{
public:
	DialogURL(std::wstring wstr);
	~DialogURL();
	WCHAR * Message;
	WCHAR * szURL;
	int nURL;
};