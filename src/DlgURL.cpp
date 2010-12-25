#include "..\vs2005\ui\resourceppc.h"
#include "DlgUrl.h"

extern HINSTANCE	g_hInst;			// current instance

BOOL DlgURLProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	static DialogURL * dlgURL;
	switch(nMsg)
	{
		case WM_INITDIALOG:
		{
			RECT rc;

			SHINITDLGINFO l_ShIDI;
			l_ShIDI.dwMask = SHIDIM_FLAGS;
			l_ShIDI.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
			l_ShIDI.hDlg = hWnd;
			SHInitDialog(&l_ShIDI);

			// MenuBar
			SHMENUBARINFO mbi;
			memset ( &mbi, 0, sizeof ( mbi ) );
			mbi.cbSize = sizeof ( mbi );
			mbi.hwndParent = hWnd;
			mbi.nToolBarId = IDR_MENU_OK_CANCEL;
			mbi.hInstRes = g_hInst;
			mbi.dwFlags |= SHCMBF_HMENU;
			SHCreateMenuBar ( &mbi );

			GetClientRect(hWnd, &rc);
			// Size the listbox control to the size of the window
			SetWindowPos(GetDlgItem(hWnd, IDC_URL_LIST), 
				0, 
				rc.left, 
				rc.top, 
				rc.right - rc.left, 
				rc.bottom - rc.top+1, 
				0);

			dlgURL = (DialogURL *)lParam;
			dlgURL->Message = wcsstr(dlgURL->Message, TEXT("http://"));
			while(dlgURL->Message)
			{
				wcscpy(dlgURL->szURL, dlgURL->Message);
				dlgURL->szURL = wcstok(dlgURL->szURL, TEXT(" \n;\,<>*\"\'\[\]\{\}"));
				dlgURL->szURL[wcslen(dlgURL->szURL)-1]='\0';
				SendMessage(GetDlgItem(hWnd, IDC_URL_LIST), LB_ADDSTRING, 0, (LPARAM)dlgURL->szURL);
				memset(dlgURL->Message, 20, 2);
				dlgURL->Message = wcsstr(dlgURL->Message, TEXT("http://"));
			}
			SendMessage(GetDlgItem(hWnd, IDC_URL_LIST), LB_SETCURSEL, 0,0);
		}
		return TRUE;
		case WM_COMMAND:
			{
				switch(LOWORD(wParam))
				{
				case IDCANCEL:
					EndDialog (hWnd, 1);
					delete dlgURL;
					return TRUE;
				case IDOK:
					{
L_OPEN_URL:
						int nSelected = SendMessage(GetDlgItem(hWnd, IDC_URL_LIST), LB_GETCURSEL, 0, 0);
						int szURLLen = SendMessage(GetDlgItem(hWnd, IDC_URL_LIST), LB_GETTEXTLEN, nSelected, 0);
						if(szURLLen)
						{
							delete dlgURL->szURL;
							dlgURL->szURL = new WCHAR[szURLLen + 1];
							SendMessage(GetDlgItem(hWnd, IDC_URL_LIST), LB_GETTEXT, nSelected, (LPARAM)dlgURL->szURL);
							
							SHELLEXECUTEINFO info;
							info.cbSize = sizeof(info);
							info.fMask = SEE_MASK_FLAG_NO_UI;
							info.hwnd = NULL;
							info.lpVerb = TEXT("open");
							info.lpFile = dlgURL->szURL;
							info.lpParameters = TEXT("");
							info.lpDirectory = TEXT("");
							info.nShow = SW_SHOW;
							ShellExecuteEx(&info);
						}
						EndDialog (hWnd, 1);
						delete dlgURL;
					}
					return TRUE;
					case IDC_URL_LIST:
					{
						switch (HIWORD (wParam))
						{
							case LBN_DBLCLK:
							{
								goto L_OPEN_URL;
							}
						}
					}
					return FALSE;
				}
			}
        break;
	}
	return FALSE;
}

DialogURL::DialogURL(std::wstring wstr)
{
	nURL = 0;
	Message = new WCHAR[wstr.length()+1];
	szURL = new WCHAR[wstr.length()+1];
	wcscpy(Message, wstr.c_str());
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_OPEN_URL), NULL, DlgURLProc, (LPARAM)this);
}

DialogURL::~DialogURL()
{
	delete Message;
	delete szURL;
}

