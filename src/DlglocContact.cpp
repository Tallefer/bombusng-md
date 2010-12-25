//#include "stdafx.h"
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>

#include "ResourceContext.h"
#include "JabberStream.h"

#include "DlglocContact.h"
#include "DlgUtils.h"

#include "JabberDataBlockListener.h"
#include "Roster.h"

#include "..\vs2005\ui\resourceppc.h"

#include "basetypes.h"
#include "utf8.hpp"
#include "stringutils.h"

extern HINSTANCE	g_hInst;			// current instance
extern RosterListView::ref rosterWnd;




INT_PTR CALLBACK DlglocContact::dialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    DlglocContact *p=(DlglocContact *) GetWindowLong(hDlg, GWL_USERDATA);

	switch (message) {

	case WM_INITDIALOG:
		{
            p=(DlglocContact *) lParam;
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

            if (p->contact) { 
                SetDlgItemText(hDlg, IDC_E_JID2, p->contact->rosterJid);

               
            }

          

            CheckDlgButton(hDlg, IDC_X_SUBSCR, BST_CHECKED);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
      

		if (LOWORD(wParam) == IDOK)
		{
            std::string jid; GetDlgItemText(hDlg, IDC_E_JID2, jid);
            std::trim(jid);
			 std::string shir; GetDlgItemText(hDlg, IDC_E_SHIR, shir);
			  std::string dolg; GetDlgItemText(hDlg, IDC_E_DOL, dolg);
			   std::string locmes; GetDlgItemText(hDlg, IDC_E_LOCMES, locmes);
            //if (!verifyJid(hDlg, jid)) return TRUE;
JabberDataBlock messageloc=("message");
messageloc.setAttribute("to",jid.c_str());
messageloc.addChild("body",locmes.c_str());
JabberDataBlockRef geoloc=messageloc.addChildNS("geoloc","http://jabber.org/protocol/geoloc");
geoloc->addChild("lat",shir.c_str());
geoloc->addChild("lon",dolg.c_str());
p->rc->jabberStream->sendStanza(messageloc);
          
              //      p->rc->sendPresence(jid.c_str(), presence::PRESENCE_AUTH_ASK, std::string(), 0);

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

void DlglocContact::createDialog(HWND parent, ResourceContextRef rc) {
    createDialog(parent, rc, Contact::ref());
}

void DlglocContact::createDialog( HWND parent, ResourceContextRef rc, Contact::ref contact ) {
    /*dlgAccountParam=accnt;*/
    DlglocContact *p=new DlglocContact();
    p->parent=parent;
    p->rc=rc;
    p->contact=contact;

   
    DialogBoxParam(g_hInst, 
        (LPCTSTR)IDD_LOC_CONTACT ,
        parent, dialogProc, (LPARAM)p);
}
