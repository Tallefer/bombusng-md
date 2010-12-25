#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberStream.h"
#include "DlgActivity.h"
#include "DlgUtils.h"

#include "JabberDataBlockListener.h"
#include "Roster.h"

#include "..\vs2005\ui\resourceppc.h"

#include "basetypes.h"
#include "utf8.hpp"
#include "stringutils.h"

extern char ***getConfig(const wchar_t *fileName,int *count);
extern HINSTANCE	g_hInst;			// current instance
extern RosterListView::ref rosterWnd;
extern std::wstring skinRootPath;
int vybor_moods2;
char ***lines2;

char *getvalue(char *str)
{
	char *str2=strchr(str,'|');
	if(str2==0) return 0;
	str2++;
	
	return str2;
}


char *getkey(char *str)
{   
	
	char *str2=strchr(str,'|');
	if(str2==0){
		return 0;}
	str2[0]='\0';


	return str;
}

INT_PTR CALLBACK DlgActivity::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DlgActivity *p=(DlgActivity *) GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:
		{vybor_moods2=0;
            p=(DlgActivity *) lParam;
            SetWindowLong(hDlg, GWL_USERDATA, (LONG) p );
			// Create a Done button and size it.  
			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN | SHIDIF_EMPTYMENU;
			shidi.hDlg = hDlg;
			SHInitDialog(&shidi);

			// MenuBar
			SHMENUBARINFO mbi;
			memset ( &mbi, 0, sizeof ( mbi ) );
			mbi.cbSize = sizeof ( mbi );
			mbi.hwndParent = hDlg;
			mbi.nToolBarId = IDR_MENU_OK_CANCEL;
			mbi.hInstRes = g_hInst;
			mbi.dwFlags |= SHCMBF_HMENU;
			SHCreateMenuBar ( &mbi );

std::wstring moodpatch=skinRootPath+TEXT("activity.txt");
    int linesCount;
	
//std::string moods_1;

	lines2=getConfig(moodpatch.c_str(),&linesCount);
	
	for (int i=0; i<linesCount; i++){ //moods_1.assign(lines[i][1]);
	SendDlgItemMessage(hDlg, IDC_ACTIV, CB_ADDSTRING, 0, (LPARAM) utf8::utf8_wchar(lines2[i][1]).c_str());}

        SendDlgItemMessage(hDlg, IDC_ACTIV, CB_SETCURSEL, vybor_moods2, 0);
        SetDlgItemText(hDlg, IDC_MES_ACTIV, L"<отсутствует текст>");

            CheckDlgButton(hDlg, IDC_X_SUBSCR, BST_CHECKED);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
      

		if (LOWORD(wParam) == IDOK)
		{
			int vybor_moods22=(int)SendDlgItemMessage(hDlg, IDC_ACTIV, CB_GETCURSEL, 0,0);
			std::string pmessage;
            GetDlgItemText(hDlg, IDC_MES_ACTIV, pmessage);
			if(vybor_moods22){if(pmessage.c_str()=="<отсутствует текст>"){pmessage=="...";}

			JabberDataBlock iq=("iq");
char* val=getvalue(lines2[vybor_moods22][0]);
char* key=getkey(lines2[vybor_moods22][0]);
iq.setAttribute("type","set");
iq.setAttribute("id",strtime::getRandom());
JabberDataBlockRef pubsub=iq.addChildNS("pubsub","http://jabber.org/protocol/pubsub");
JabberDataBlockRef publish=pubsub->addChild("publish");
publish->setAttribute("node", "http://jabber.org/protocol/activity");
JabberDataBlockRef item=publish->addChild("item");
JabberDataBlockRef mood=item->addChildNS("activity","http://jabber.org/protocol/activity");
JabberDataBlockRef tegmood=mood->addChild(key,NULL);
JabberDataBlockRef tegact=tegmood->addChild(val,NULL);
JabberDataBlockRef text=mood->addChild("text",pmessage.c_str());

p->rc->jabberStream->sendStanza(iq);
			}else{
			
			JabberDataBlock iq=("iq");

iq.setAttribute("type","set");
iq.setAttribute("id",strtime::getRandom());
JabberDataBlockRef pubsub=iq.addChildNS("pubsub","http://jabber.org/protocol/pubsub");
JabberDataBlockRef publish=pubsub->addChild("publish");
publish->setAttribute("node", "http://jabber.org/protocol/activity");
JabberDataBlockRef item=publish->addChild("item");
JabberDataBlockRef tune=item->addChildNS("activity","http://jabber.org/protocol/activity");

p->rc->jabberStream->sendStanza(iq);
			}
            

			EndDialog(hDlg, LOWORD(wParam));
            delete p;
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
            delete p;
			return TRUE;
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, message);
        delete p;
		return TRUE;
	}
	return (INT_PTR)FALSE;
}

void DlgActivity::createDialog(HWND parent, ResourceContextRef rc) {
    createDialog(parent, rc, Contact::ref());
}

void DlgActivity::createDialog( HWND parent, ResourceContextRef rc, Contact::ref contact ) {
    /*dlgAccountParam=accnt;*/
    DlgActivity *p=new DlgActivity();
    p->parent=parent;
    p->rc=rc;
    p->contact=contact;

   
    DialogBoxParam(g_hInst, 
        (LPCTSTR)IDD_ACTIVITY ,
        parent, dialogProc, (LPARAM)p);
}
