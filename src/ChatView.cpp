#include "ChatView.h"
#include <utf8.hpp>
#include <commctrl.h>
#include <windowsx.h>
#include <aygshell.h>
#include <sipapi.h>
#include "../vs2005/ui/ui.h"

#include "Sysinfo.h"
#include "wmuser.h"
#include "ResourceContext.h"
#include "JabberStream.h"
#include "JabberAccount.h"
#include "TabCtrl.h"
#include "ProcessMUC.h"

#include "Smiles.h"
#include "History.h"

#include "LastActivity.h"

#include "config.h"

#include "stringutils.h"
#include "VcardForm.h"
#include <string.h>
#include "Log.h"
#include <windows.h>
#include <Wingdi.h>

#include "DlgURL.h"

extern int smile_aktiv;
extern std::wstring appRootPath;
extern HINSTANCE			g_hInst;
extern int tabHeight;
extern HWND	g_hWndMenuBar;		// menu bar handle
extern ResourceContextRef rc;
extern ImgListRef skin;
extern SmileParser *smileParser;
extern HWND		mainWnd;
extern TabsCtrlRef tabs;			/* to vCard ICO */
HMENU g_hnicknames;
extern LONG timstatus;
extern int idautostatus;
extern char ***strcom;
extern int linesCountcom;
extern int smile_anim_ind;
extern bool nofocus;
//LONG tolshina=400;
//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////
#define GOTOURL   1225
#define COMMAND_STR   52205
std::wstring filePathavatargo;
LONG avWidthgo;
LONG avHeightgo;
bool mucak=0;
//HWND chatHWND;
void ExecFile(LPCTSTR link, LPCTSTR param)
{
   SHELLEXECUTEINFO info;
   info.cbSize = sizeof(info);
   info.fMask = SEE_MASK_FLAG_NO_UI;
   info.hwnd = NULL;
   info.lpVerb = L"open";
   info.lpFile = link;
   info.lpParameters = param;
   info.lpDirectory = L"";
   info.nShow = SW_SHOW;
   //info.hInstApp = AfxGetInstanceHandle(); // можно не заполнять данное поле, и так работает
   ShellExecuteEx(&info);
}
LOGFONTW FONT;    //
HFONT NormalFont;
//ExecFile(L"http://ya.ru",L"");
//////////////////////////////////////////////////////////////////////////
ATOM ChatView::RegisterWindowClass() {
    WNDCLASS wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = ChatView::WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = 0;
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = _T("BombusCV");

    return RegisterClass(&wc);
}

bool avatbool;

//////////////////////////////////////////////////////////////////////////
// real WndProc for edit box
//long (WINAPI *EditWndProc)(HWND w,UINT msg,WPARAM wParam,LPARAM lParam);

namespace editbox {
    static bool editBoxShifts=false;
}
//wchar_t *strcom[5]={L"/me",L"/cry",L"/dance",L"/liver",L"/proba",};
long WINAPI EditSubClassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WNDPROC OldWndProc=(WNDPROC) GetWindowLong(hWnd, GWL_USERDATA);

	switch(msg) {
    case WM_LBUTTONDOWN:
        {
            SHRGINFO    shrg;
            shrg.cbSize = sizeof(shrg);
            shrg.hwndClient = hWnd;
            shrg.ptDown.x = LOWORD(lParam);
            shrg.ptDown.y = HIWORD(lParam);
            shrg.dwFlags = SHRG_RETURNCMD /*| SHRG_NOANIMATION*/;

			if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU) {

                DWORD sel=SendMessage(hWnd, EM_GETSEL, 0, 0);

                UINT paste = (IsClipboardFormatAvailable(CF_UNICODETEXT))?  MF_STRING : MF_STRING | MF_GRAYED;
                UINT cut = (LOWORD(sel)!=HIWORD(sel))? MF_STRING : MF_STRING | MF_GRAYED;
                UINT undo= (SendMessage(hWnd, EM_CANUNDO, 0, 0))? MF_STRING : MF_STRING | MF_GRAYED;;
                HMENU hmenu = CreatePopupMenu();
                if (hmenu==NULL) break;
                HMENU subscrMenupCom=CreatePopupMenu();//подменю команд
				HMENU subscrMenupJUIC=CreatePopupMenu();
				AppendMenu(subscrMenupJUIC, MF_STRING, 70000, TEXT("последние 10") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70001, TEXT("последние 10 поп.") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70002, TEXT("поп. теги") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70003, TEXT("поп. блоги") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70004, TEXT("удалить последний пост") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70005, TEXT("логин") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70006, TEXT("пинг") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70007, TEXT("обн.инф. из vcard") );

				if(linesCountcom)for(int r=0;r<linesCountcom;r=r+3)AppendMenu(subscrMenupCom, MF_STRING,r+40000 , utf8::utf8_wchar(strcom[r][1]).c_str());//добавляем все подпункты

				//Собственно окно набора сообщения
                AppendMenu(hmenu, (smileParser->hasSmiles())? MF_STRING : MF_STRING | MF_GRAYED, ADD_SMILE, TEXT("Смайлы"));
				AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenupCom, TEXT("Команды"));
                AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenupJUIC, TEXT("juick"));
				AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hmenu, cut, WM_CUT, TEXT("Вырезать") );
                AppendMenu(hmenu, cut, WM_COPY, TEXT("Копировать") );
                AppendMenu(hmenu, paste, WM_PASTE, TEXT("Вставить") );
                AppendMenu(hmenu, undo, EM_UNDO, TEXT("Отмена") );
				AppendMenu(hmenu, MF_STRING, CLEARME, TEXT("Очистить") );
				AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hmenu, MF_STRING, SB_, TEXT("ТЕМА") );

                POINT pt={LOWORD(lParam), HIWORD(lParam) };
                ClientToScreen(hWnd, &pt);

                int cmdId=TrackPopupMenuEx(hmenu,
                    TPM_BOTTOMALIGN | TPM_RETURNCMD,
                    pt.x, pt.y,
                    hWnd,
                    NULL);

                if (cmdId==ADD_SMILE) PostMessage(GetParent(hWnd), WM_COMMAND, 8000, 0);//SmileBox::showSmileBox(hWnd, pt.x, pt.y, smileParser);
				 if (cmdId==SB_)PostMessage(GetParent(hWnd), WM_COMMAND, IDC_SB, 0);
                DestroyMenu(hmenu);
				if(cmdId>=40000 && cmdId<=40000 +linesCountcom)PostMessage(GetParent(hWnd), COMMAND_STR, cmdId-40000, 0);
                if(cmdId>=70000 && cmdId<=70007)PostMessage(GetParent(hWnd), JUICK_COMMAND, cmdId-70000, 0);
				if (cmdId==CLEARME)PostMessage(GetParent(hWnd), WM_COMMAND, CLEARED, 0);
				if (cmdId>0) PostMessage(hWnd, cmdId, 0, 0);

                return 0;
            }
            break;
        }

    case WM_KEYDOWN:
		smile_aktiv=40 ;
        if (wParam==VK_CONTROL) editbox::editBoxShifts=true;
        if (wParam==VK_SHIFT)   editbox::editBoxShifts=true;
		if ((wParam==VK_LEFT) && (SendMessage (hWnd, EM_LINELENGTH,(WPARAM)0, (LPARAM)0)==0)) PostMessage(GetParent(GetParent(hWnd)), WM_COMMAND, TabsCtrl::PREVTAB, 0);
		if ((wParam==VK_RIGHT) && (SendMessage (hWnd, EM_LINELENGTH,(WPARAM)0, (LPARAM)0)==0))  PostMessage(GetParent(GetParent(hWnd)), WM_COMMAND, TabsCtrl::NEXTTAB, 0);
		if (wParam==VK_UP) //выводит фокус обратно в сообщения
		{
			//этот способ реализации подсказал skipyrich :)
			int ndx1 = 0, ndx2 = 0;
			SendMessage (hWnd, EM_GETSEL, (WPARAM)&ndx1, (LPARAM)&ndx2);
			int len = SendMessage (hWnd, EM_LINELENGTH,(WPARAM)0, (LPARAM)0);	//вычисляем длину строки
			if (ndx1<=len) //отправляем сообщение о необходимости выделить MessageList
				PostMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)MSGListFocus, (LPARAM)0);
		}

        break;

    case WM_KEYUP:
        editbox::editBoxShifts=false;
        break;
	case CLEARMESS:{
			PostMessage(GetParent(hWnd), CLEARMESS, NULL, false);
			return true;}

	case JUICK_SEND:{
			PostMessage(GetParent(hWnd), WM_COMMAND, IDS_SEND, 0);
			return 0;}


    case WM_CHAR:
        if (wParam==VK_RETURN && !editbox::editBoxShifts)
		{
			SHRGINFO    shrg;
            shrg.cbSize = sizeof(shrg);
            shrg.hwndClient = hWnd;
            shrg.ptDown.x = LOWORD(lParam);
            shrg.ptDown.y = HIWORD(lParam);
            shrg.dwFlags = SHRG_RETURNCMD | SHRG_NOANIMATION;

			if (SHRecognizeGesture(&shrg) == GN_CONTEXTMENU)
			{
				DWORD sel=SendMessage(hWnd, EM_GETSEL, 0, 0);

                UINT paste = (IsClipboardFormatAvailable(CF_UNICODETEXT))?  MF_STRING : MF_STRING | MF_GRAYED;
                UINT cut = (LOWORD(sel)!=HIWORD(sel))? MF_STRING : MF_STRING | MF_GRAYED;
                UINT undo= (SendMessage(hWnd, EM_CANUNDO, 0, 0))? MF_STRING : MF_STRING | MF_GRAYED;;
                HMENU hmenu = CreatePopupMenu();
                if (hmenu==NULL) break;
                HMENU subscrMenupCom=CreatePopupMenu();//подменю команд
				HMENU subscrMenupJUIC=CreatePopupMenu();
				AppendMenu(subscrMenupJUIC, MF_STRING, 70000, TEXT("последние 10") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70001, TEXT("последние 10 поп.") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70002, TEXT("поп. теги") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70003, TEXT("поп. блоги") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70004, TEXT("удалить последний пост") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70005, TEXT("логин") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70006, TEXT("пинг") );
				AppendMenu(subscrMenupJUIC, MF_STRING, 70007, TEXT("обн.инф. из vcard") );

				if(linesCountcom)for(int r=0;r<linesCountcom;r=r+3)AppendMenu(subscrMenupCom, MF_STRING,r+40000 , utf8::utf8_wchar(strcom[r][1]).c_str());//добавляем все подпункты

				//Собственно окно набора сообщения
                AppendMenu(hmenu, (smileParser->hasSmiles())? MF_STRING : MF_STRING | MF_GRAYED, ADD_SMILE, TEXT("Смайлы"));
				AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenupCom, TEXT("Команды"));
                AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hmenu, MF_POPUP, (LPARAM)subscrMenupJUIC, TEXT("juick"));
				AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hmenu, cut, WM_CUT, TEXT("Вырезать") );
                AppendMenu(hmenu, cut, WM_COPY, TEXT("Копировать") );
                AppendMenu(hmenu, paste, WM_PASTE, TEXT("Вставить") );
                AppendMenu(hmenu, undo, EM_UNDO, TEXT("Отмена") );
				AppendMenu(hmenu, MF_STRING, CLEARME, TEXT("Очистить") );
				AppendMenu(hmenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hmenu, MF_STRING, SB_, TEXT("ТЕМА") );

                POINT pt={LOWORD(lParam), HIWORD(lParam) };
                ClientToScreen(hWnd, &pt);

                int cmdId=TrackPopupMenuEx(hmenu,
                    TPM_BOTTOMALIGN | TPM_RETURNCMD,
                    pt.x, pt.y,
                    hWnd,
                    NULL);

                if (cmdId==ADD_SMILE) PostMessage(GetParent(hWnd), WM_COMMAND, 8000, 0);//SmileBox::showSmileBox(hWnd, pt.x, pt.y, smileParser);
				 if (cmdId==SB_)PostMessage(GetParent(hWnd), WM_COMMAND, IDC_SB, 0);
                DestroyMenu(hmenu);
				if(cmdId>=40000 && cmdId<=40000 +linesCountcom)PostMessage(GetParent(hWnd), COMMAND_STR, cmdId-40000, 0);
                if(cmdId>=70000 && cmdId<=70007)PostMessage(GetParent(hWnd), JUICK_COMMAND, cmdId-70000, 0);
				if (cmdId==CLEARME)PostMessage(GetParent(hWnd), WM_COMMAND, CLEARED, 0);
				if (cmdId>0) PostMessage(hWnd, cmdId, 0, 0);

                return 0;
			}
            if (Config::getInstance()->enter2)
			{
				int ndx1 = 0, ndx2 = 0;
				SendMessage (hWnd, EM_GETSEL, (WPARAM)&ndx1, (LPARAM)&ndx2); // возвращяет позиции символом с которых начинается и заканчивается выделение
				int len = SendMessage (hWnd, EM_LINELENGTH, (WPARAM)-1, (LPARAM)0); // длина текста в текущей строке
				int nlines = SendMessage (hWnd, EM_GETLINECOUNT, (WPARAM)0, (LPARAM)0); // сколько всего линий
				int cline = SendMessage (hWnd, EM_LINEINDEX, (WPARAM)nlines+1, (LPARAM)0); // тут минус -1 когда след строки не существует что нам и надо
				if ((ndx1 == ndx2) & (len == 0) & (cline == -1)) { // проверяем чтобы небыло выделения, длинна = 0 и следующий строки не существует
					PostMessage(GetParent(hWnd), WM_COMMAND, IDS_SEND, 0);
					return 0;
				}
			} else {
					PostMessage(GetParent(hWnd), WM_COMMAND, IDS_SEND, 0);
					return 0;
			}

        }
        if (wParam==VK_TAB) {
            PostMessage(GetParent(hWnd), WM_COMMAND, IDC_COMPLETE, 0);
            return 0;
        }
        if (wParam>=' ')
            PostMessage(GetParent(hWnd), WM_COMMAND, IDC_COMPOSING, true);
        break;
    case WM_SETFOCUS:
        if (Config::getInstance()->raiseSIP) SipShowIM(SIPF_ON);
        break;
    case WM_KILLFOCUS:
        if (Config::getInstance()->raiseSIP) SipShowIM(SIPF_OFF);
        PostMessage(GetParent(hWnd), WM_COMMAND, IDC_COMPOSING, false);
        break;
    }
    return CallWindowProc(OldWndProc,hWnd,msg,wParam,lParam);
}

////////////////////////////////////////////////////////////////////////////////
HWND WINAPI DoCreateEditControl(HWND hwndParent) {

    HWND hWndEdit;
    //TCITEM tie;

    hWndEdit=CreateWindow(_T("EDIT"),
		NULL,
        WS_BORDER| WS_CHILD | WS_VISIBLE | WS_VSCROLL
        | ES_MULTILINE ,
        0, 0, //x,y начало
		CW_USEDEFAULT, CW_USEDEFAULT, //ширина и высота автоматом
        hwndParent, NULL, g_hInst, NULL);

    WNDPROC OldEditWndProc = (WNDPROC)SetWindowLong(hWndEdit, GWL_WNDPROC,  (LONG)EditSubClassProc);
    SetWindowLong(hWndEdit, GWL_USERDATA, (LONG)OldEditWndProc);
    return hWndEdit;
}

std::wstring toWString(const std::string& s)
{
	std::wstring temp(s.length(),L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

//const wchar_t * MessageElement::getInfo() const { return wstr.c_str(); }


//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK ChatView::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
    PAINTSTRUCT ps;
    HDC hdc;
    ChatView *p=(ChatView *) GetWindowLong(hWnd, GWL_USERDATA);

    switch (message) {
    case WM_CREATE:
		{
            p=(ChatView *) (((CREATESTRUCT *)lParam)->lpCreateParams);
            SetWindowLong(hWnd, GWL_USERDATA, (LONG) p );

            p->msgList=VirtualListView::ref(new VirtualListView(hWnd, std::string("Chat")));
            p->msgList->setParent(hWnd);
			nofocus=0;
            p->msgList->showWindow(true);
            p->msgList->wrapList=false;
            p->msgList->colorInterleaving=true;

			p->editWnd=p->msgList->hEditBox=DoCreateEditControl(hWnd);
            p->calcEditHeight();

            p->msgList->bindODRList(p->contact->messageList);

            break;
        }

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

		{
            //p->contact->nUnread=0;
			int avataraWidth = Config::getInstance()->avatarWidth ;
			bool muc=boost::dynamic_pointer_cast<MucRoom>(p->contact);
			mucak=muc;
			 /*if(muc){
			   avataraWidth = 0;
			 }*/
			 RECT rc ;
			rc.top=0;rc.left=0;
			if (!sysinfo::screenIsVGA()) {
					rc.right=200;
					rc.bottom=(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:50) ;
			} else {
					rc.right=390;
					rc.bottom=(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:50) ;
			}
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, p->contact->getColor());
            //p->contact->draw(hdc, rc);
			LONG avWidth=0;
			LONG avHeight=0;
			int iconIdx=p->contact->getIconIndex();
			if(!muc){
				std::string avatarsjid=p->contact->rosterJid;
				size_t i2=0;
                while (i2<avatarsjid.length()) {
                    if (avatarsjid[i2]=='/') {
						avatarsjid[i2]='.';
                        continue;
                    }
                    i2++;
				}
			std::wstring filePathavatar6=appRootPath+L"userdata\\avatars\\"+utf8::utf8_wchar(avatarsjid)+L".jpg";
			HBITMAP bmp4=SHLoadImageFile(filePathavatar6.c_str());
			filePathavatargo=filePathavatar6;
			//int result2=MessageBox(NULL, filePathavatar6.c_str(), TEXT("Открыть"), MB_YESNOCANCEL | MB_ICONWARNING );
			BITMAP bm4;
			GetObject(bmp4, sizeof(bm4), &bm4);
			LONG avataraWidthL = Config::getInstance()->avatarWidth ;
			avatbool=0;
			if (bmp4) {
				avatbool=1;
				if(bm4.bmWidth==bm4.bmHeight){avWidth=avataraWidthL;
				avHeight=avWidth;
				} else {
				if(bm4.bmWidth>bm4.bmHeight)
				{
					avWidth=avataraWidthL;
					avHeight=(avataraWidthL*bm4.bmHeight*100)/(100*bm4.bmWidth);
				}else{
					avHeight=avataraWidthL;
					avWidth=(avataraWidthL*bm4.bmWidth*100)/(100*bm4.bmHeight);
				}}

				HDC hdcImage2=CreateCompatibleDC(NULL);
				SelectObject(hdc, bmp4);
				SelectObject(hdcImage2, bmp4);
				COLORREF transparentColor2=GetPixel(hdcImage2, 0, 0);DeleteDC(hdcImage2);
				avWidthgo= avWidth;avHeightgo=avHeight;
				TransparentImage(hdc,rc.top,rc.left,avWidth, avHeight, bmp4, 0, 0,  bm4.bmWidth, bm4.bmHeight, transparentColor2);
				rc.left+=avWidth+1;
			}
			if (bmp4) DeleteObject(bmp4);

			}

			if (iconIdx>=0) {
				skin->drawElement(hdc, p->contact->getIconIndex(), rc.left, rc.top);
				rc.left+=skin->getElementWidth();//Ширина элемента
			} else rc.left+=1;

			//LOGFONTW FONT;
			strcpy((char*)FONT.lfFaceName, "Tahoma");
			FONT.lfHeight = Config::getInstance()->msg_font_height;
			FONT.lfWidth = Config::getInstance()->msg_font_width;
			FONT.lfWeight = Config::getInstance()->tolshina;
			FONT.lfItalic = false;
			FONT.lfStrikeOut = false;
			FONT.lfUnderline = false;
			FONT.lfOrientation = 0;
			FONT.lfEscapement = 0;
			NormalFont  = CreateFontIndirectW(&FONT);
			SelectObject(hdc, NormalFont);
			DrawText(hdc, p->contact->getText(), -1, &rc, DT_LEFT | DT_TOP | DT_END_ELLIPSIS);


            int iconwidth= skin->getElementWidth();//Получаем ширину иконки

			if(!muc)
			{//Если конфа,убираем инфо панель
				if(Config::getInstance()->confclient){
				int clientIcon;
				Skin * il= dynamic_cast<Skin *>(skin.get());
				std::string ClientI=p->contact->getClientIdIcon();

				if(ClientI.length()>2){if (il) clientIcon=il->getKlientIndex((char*)ClientI.c_str());}else clientIcon=0;

					if(clientIcon)skin->drawElement(hdc, clientIcon, avWidth , iconwidth);
				}

				/*ExtTextOut(hdc, avWidth + iconwidth + 4, iconwidth, NULL, NULL,
					utf8::utf8_wchar(p->contact->getClientIdIcon()).c_str(),
					p->contact->getClientIdIcon().length(),
					NULL);
				*/
				std::wstring statuschat=utf8::utf8_wchar(p->contact->getStatusMessage());
				ExtTextOut(hdc, avWidth + iconwidth + 4, iconwidth, NULL, NULL,
					statuschat.c_str(),
					statuschat.length(),
					NULL);

			}

            skin->drawElement(hdc, icons::ICON_CLOSE,p->width-2-iconwidth, 0);
            skin->drawElement(hdc, icons::ICON_TRASHCAN_INDEX,p->width-2-iconwidth*2, 0);
            if(!muc)
			{skin->drawElement(hdc, icons::ICON_VCARD,p->width-2-iconwidth*3,0);}  // vCard in Tab//не нужен в конфе
			DeleteObject(NormalFont);

		}

        EndPaint(hWnd, &ps);
        break;

    //case WM_KILLFOCUS:
    //    p->contact->nUnread=0;
    //    break;

    case WM_SIZE:
		{
            HDWP hdwp;
            RECT rc;
//
//#	уменьшаем поле ввода в ландшафтной ориентации
//#
            int height=GET_Y_LPARAM(lParam);
            p->width=GET_X_LPARAM(lParam);
            //int ySplit=height-p->editHeight;
// добавить опуцию в настройки :)
			int ySplit;
				if(!Config::getInstance()->editx2){ySplit=//(sysinfo::screenIsRotate())?height-p->editHeight/2 :
					height-p->editHeight/2;}else{ySplit=height-p->editHeight;}
			//Оставляем поле ввода сообщения в 2 раза меньше оригинала

            p->calcEditHeight();

            // Calculate the display rectangle, assuming the
            // tab control is the size of the client area.
            SetRect(&rc, 0, 0,
                GET_X_LPARAM(lParam), ySplit );

            // Size the tab control to fit the client area.
            hdwp = BeginDeferWindowPos(2);

            /*DeferWindowPos(hdwp, dropdownWnd, HWND_TOP, 0, 0,
            GET_X_LPARAM(lParam), 20,
            SWP_NOZORDER
            ); */


			DeferWindowPos(hdwp, p->msgList->getHWnd(), HWND_TOP, 0, sysinfo::screenIsVGA()?(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:65):(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:50),
                GET_X_LPARAM(lParam), ySplit-(sysinfo::screenIsVGA()?(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:65):(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:50)),
                SWP_NOZORDER
                );
            /*DeferWindowPos(hdwp, rosterWnd, HWND_TOP, 0, tabHeight,
            GET_X_LPARAM(lParam), height-tabHeight,
            SWP_NOZORDER
            );*/

            DeferWindowPos(hdwp, p->editWnd, NULL, 0, ySplit+1,
                GET_X_LPARAM(lParam), height-ySplit-1,
                SWP_NOZORDER
                );

            EndDeferWindowPos(hdwp);

            break;
        }
	case COMMAND_STR://обработка вставки быстрых команд
		{p->comstr((int)wParam);break;}
	case JUICK_COMMAND:{
		if((int)wParam==0)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"#+");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==1)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"#");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==2)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"*");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==3)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"@");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==4)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"D L");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==5)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"LOGIN");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==6)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"PING");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}
		if((int)wParam==7)
			{SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"VCARD");
			SendMessage(p->editWnd, JUICK_SEND, 0, 0);}

					   }



    case WM_COMMAND:
		{   if( wParam==8000){SmileBox::showSmileBox(p->editWnd, 0, 0,p->msgList->getHWnd(), smileParser);
		}
            if (wParam==IDS_SEND) {
                p->sendJabberMessage();
            }
			if (wParam==IDC_SB) {
                p->sendJabberMessagesb();
            }
            if (wParam==IDC_COMPLETE) {
                p->mucNickComplete();
            }

            if (wParam==IDC_COMPOSING) {
                p->setComposingState(lParam!=0);
            }
			if (wParam==CLEARED) {
                SendMessage(p->editWnd, WM_SETTEXT, 1, (LPARAM) L"");
            }

			if (wParam==MSGListFocus) {
				/*
				ODRRef a = p->msgList->getCursorPos();
				// определить что a несуществует
				// ено
				// иначе приходится два раза нажиматтоесть ничего не выдель чтобы было видно курсор в поле
				if (a ???? )
				{
					p->msgList->moveCursorEnd();
					p->msgList->notifyListUpdate(true);
				}
				*/
				SendMessage(p->msgList->getHWnd(), MSGListFocus, 0, 0);
			}

            break;
        }
        /*case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT:
        {
        //HGDIOBJ brush= GetStockObject(GRAY_BRUSH);
        //HGDIOBJ pen= GetStockObject(WHITE_PEN);
        SetBkColor(hdc, 0x808080);
        SetTextColor(hdc, 0xffffff);
        //SelectObject((HDC)wParam, brush);
        //SelectObject((HDC)wParam, pen);
        return (BOOL) GetStockObject(GRAY_BRUSH);
        break;

		skin->drawElement(hdc, icons::ICON_BOMBUS_NG, avatarWidth, iconwidth);



		skin->drawElement(hdc, icons::ICON_CLOSE, avatarWidth,iconwidth*2);
		skin->drawElement(hdc, icons::ICON_TRASHCAN_INDEX,p->width-2-iconwidth, 0);

		skin->drawElement(hdc, icons::ICON_TRASHCAN_INDEX,avatarWidth+iconwidth*2+10,iconwidth*2 );
		skin->drawElement(hdc, icons::ICON_TRASHCAN_INDEX,p->width-2-(skin->getElementWidth())*2, 0);

		skin->drawElement(hdc, icons::ICON_VCARD, avatarWidth+iconwidth*3+10,iconwidth*2);         p->width-2-iconwidth*3,0  // vCard in Tab

        }*/
	case CLEARMESS:
		{int result=MessageBox(
				p->getHWnd(),
				L"Очистить окно от сообщений?",
				L"Очистить",
				MB_YESNO | MB_ICONWARNING);
			if (result==IDYES) {
				p->contact->messageList->clear();
				p->msgList->moveCursorEnd();
			}
			break;
				   }
	case WM_LBUTTONDOWN:
		SetFocus(hWnd);
		if ((GET_Y_LPARAM(lParam))>(boost::dynamic_pointer_cast<MucRoom>(p->contact)?tabHeight:50)) break; //если конфа,ставим tabHeight

		if (GET_X_LPARAM(lParam) > (p->width)-2-(skin->getElementWidth())) {
			PostMessage(GetParent(hWnd), WM_COMMAND, TabsCtrl::CLOSETAB, 0);
			break;
		}
		if (GET_X_LPARAM(lParam) > (p->width)-2-(skin->getElementWidth())*2) {
			int result=MessageBox(
				p->getHWnd(),
				L"Очистить окно от сообщений?",
				L"Очистить",
				MB_YESNO | MB_ICONWARNING);
			if (result==IDYES) {
				p->contact->messageList->clear();
				p->msgList->moveCursorEnd();
			}

			//if(!(boost::dynamic_pointer_cast<MucRoom>(p->contact))){

			//}
			break;
		}
if(!boost::dynamic_pointer_cast<MucRoom>(p->contact))
{if(avatbool){
		if (GET_X_LPARAM(lParam)<avWidthgo && GET_X_LPARAM(lParam)>0 ){
					//MessageBox(NULL, TEXT("avatar"), TEXT("proba"), 0);
			ExecFile(filePathavatargo.c_str(),L"");break;}}
		if (GET_X_LPARAM(lParam) > (p->width)-2-(skin->getElementWidth())*3) {
				if (!rc->isLoggedIn()) break;
				WndRef vc=VcardForm::createVcardForm(tabs->getHWnd(),p->contact->rosterJid, rc, false);
				//Запрос визитки
				tabs->addWindow(vc);
				tabs->switchByWndRef(vc);
                break;
		}
}

    case WM_DESTROY:
        //TODO: Destroy all child data associated eith this window

        return 0;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

ChatView::ChatView( HWND parent, Contact::ref contact )
{
    BOOST_ASSERT(parent);

    if (windowClass==0)
        windowClass=RegisterWindowClass();
    if (windowClass==0) throw std::exception("Can't create window class");

    parentHWnd=parent;

    //TEXTMETRIC txm;
    //GetTextMetrics(NULL, &txm); //TODO - hdc must not be NULL
    //editHeight=txm.tmHeight*4;
    editHeight=(sysinfo::screenIsVGA())? 50*2 : 50;

    this->contact=contact;

    thisHWnd=CreateWindow((LPCTSTR)windowClass, _T("chat"), WS_CHILD |WS_VISIBLE,
        0, 0,
		CW_USEDEFAULT, CW_USEDEFAULT,//ширина+высота
		parent, NULL, g_hInst, (LPVOID)this);

}

//const wchar_t * ChatView::getWindowTitle() const{  return TEXT("stub"); }

ChatView::~ChatView() {
    DestroyWindow(editWnd);
}

const ODR * ChatView::getODR() const { return contact.get(); }

void ChatView::addMessage(const std::string & msg) {
    /*std::wstring umsg=utf8::utf8_wchar(msg);
    ODRRef r=ODRRef(new IconTextElementContainer(umsg, -1));*/
    ODRRef r=ODRRef(new MessageElement(msg));
    msgList->addODR(r, true);
}

bool ChatView::showWindow( bool show ) {
    bool oldState=Wnd::showWindow(show);
    if (oldState!=show) contact->nUnread=0;

    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_STATE;
    tbbi.fsState = (show)? TBSTATE_ENABLED : TBSTATE_HIDDEN;

    ::SendMessage (g_hWndMenuBar, TB_SETBUTTONINFO, IDS_SEND, (LPARAM)&tbbi);

    if (show) contact->nUnread=0;

    if (show) msgList->notifyListUpdate(true);
    //if (show) InvalidateRect(msgList->getHWnd(), NULL, false);

  if (show && !nofocus) SetFocus(editWnd);//нефокусировать при перерисовке по таймеру

    return oldState;
}

void ChatView::redraw(){
    InvalidateRect(getHWnd(), NULL, TRUE);
    msgList->notifyListUpdate(true);
    //InvalidateRect(msgList->getHWnd(), NULL, false);
}

void ChatView::moveEnd() {
    msgList->moveCursorEnd();
}

bool ChatView::autoScroll() {
    if (!IsWindowVisible(getHWnd())) return false;
    return (msgList->cursorAtEnd());
}

ATOM ChatView::windowClass=0;

//////////////////////////////////////////////////////////////////////////

void ChatView::comstr(int comnum){//вставляем команду

	SendMessage(editWnd, EM_REPLACESEL, TRUE, (LPARAM)utf8::utf8_wchar(strcom[(comnum)?comnum:comnum+1][0]).c_str());
					}

void ChatView::sendJabberMessage() {
    int len=SendMessage(editWnd, WM_GETTEXTLENGTH, 0, 0);
    if (len==0) return;
    len+=1; //null-terminating char

    wchar_t *buf=new wchar_t[len+1];
    int actualLen=SendMessage(editWnd, WM_GETTEXT, len, (LPARAM) buf);
    std::string body=utf8::wchar_utf8(buf);
    delete[] buf;

    std::trimTail(body);
    if (body.length()==0) return;
	bool muc=boost::dynamic_pointer_cast<MucRoom>(contact);
	if (!muc){body.insert(0,"ME>");}// Свои сообщения в привате отображать с ME> в начале чтобы было понятно где сам писал
timstatus=0;
if(idautostatus==1)idautostatus=2;
    Message::ref msg=Message::ref(new Message(body, rc->account->getNickname(), false, Message::SENT, strtime::getCurrentUtc() ));

if (Config::getInstance()->history)
{if (!muc){
			History::getInstance()->appendHistory(contact, msg,false);}}
if (Config::getInstance()->saveHistoryMuc){
		if (!muc){
			contact->messageList->push_back(msg);

		}
       // History::getInstance()->appendHistory(contact, msg,true);
	}else{
       if (!muc) {
         contact->messageList->push_back(msg);

       }
	}
if(!muc)msg->body.erase(0,3);//перед отправкой убираем ME>
    msgList->moveCursorEnd();
    redraw();

    //if (!muc) msgList->moveCursorEnd();

    std::string to=(muc)?contact->jid.getBareJid() : contact->jid.getJid();
    JabberDataBlockRef out=msg->constructStanza(to);
    if (muc) out->setAttribute("type","groupchat");
    else {
        /* xep-0022; deprecated; will be re-enabled with entity caps
        JabberDataBlockRef x=out->addChildNS("x", "jabber:x:event");
        x->addChild("delivered", NULL);
        x->addChild("composing", NULL);
        */

        if (Config::getInstance()->composing)
            out->addChildNS("active","http://jabber.org/protocol/chatstates");

        if (Config::getInstance()->delivered)
            //if (! contact->jid.getResource().empty() )
            out->addChildNS("request","urn:xmpp:receipts");
    }
    composing=false;

    //Reset form
    rc->jabberStream->sendStanza(*out);

    LastActivity::update();

    SendMessage(editWnd, WM_SETTEXT, 1, (LPARAM) L"");
	if (Config::getInstance()->sip2)SipShowIM(SIPF_OFF);
}


void ChatView::sendJabberMessagesb() {
    int len=SendMessage(editWnd, WM_GETTEXTLENGTH, 0, 0);
    if (len==0) return;
    len+=1; //null-terminating char

    wchar_t *buf=new wchar_t[len+1];
    wchar_t *buf2=new wchar_t[len+12];

    int actualLen=SendMessage(editWnd, WM_GETTEXT, len, (LPARAM) buf);
	wcscpy(buf2,L"Тема:");
	wcscat(buf2,(const wchar_t*)buf);


    std::string body=utf8::wchar_utf8(buf2);
    delete[] buf;
	delete[] buf2;

    std::trimTail(body);
    if (body.length()==0) return;
    Message::ref msg=Message::ref(new Message(body, rc->account->getNickname(), false, Message::SENT, strtime::getCurrentUtc() ));
    bool muc=boost::dynamic_pointer_cast<MucRoom>(contact);
	if (!muc)return;


    msgList->moveCursorEnd();
    redraw();

    	timstatus=0;
if(idautostatus==1)idautostatus=2;
    std::string to=contact->jid.getBareJid();
    JabberDataBlockRef out=msg->constructStanzasb(to);



    rc->jabberStream->sendStanza(*out);

    LastActivity::update();

    SendMessage(editWnd, WM_SETTEXT, 1, (LPARAM) L"");
	if (Config::getInstance()->sip2)SipShowIM(SIPF_OFF);
}

void ChatView::calcEditHeight() {
    RECT rect;
    GetWindowRect(editWnd, &rect);
}

//////////////////////////////////////////////////////////////////////////
bool nickCompare( std::wstring left, std::wstring right ) {
    return (_wcsicmp(left.c_str(), right.c_str()) < 0);
}

void ChatView::mucNickComplete() {
    //step 1 - verify if this chat is muc-chat
    MucGroup::ref roomGrp;
    roomGrp=boost::dynamic_pointer_cast<MucGroup> (rc->roster->findGroup(contact->jid.getBareJid()));
    if (!roomGrp) return;
    if (roomGrp->room!=contact) return;

    //step 2 - extracting data for autocomplete
    wchar_t buf[1024];
    int len=SendMessage(editWnd, WM_GETTEXT, 1024, (LPARAM) buf);
    size_t mbegin;
    size_t mend;
    SendMessage(editWnd, EM_GETSEL, (WPARAM)&mbegin, (LPARAM)&mend);

    //step 3 - search nick begin and end
    size_t nbegin=mbegin;
    while (nbegin>0) {
        nbegin--;
        if (iswspace(buf[nbegin])) { nbegin++; break; }
    }

    size_t nend=mend;
    while (nend>mbegin) {
        nend--;
        if (buf[nend]==':') continue;
        if (!iswspace(buf[nend])) { nend++; break; }
    }

    //now: [nbegin..mbegin) - constant part (case may be altered)
    size_t clen=mbegin-nbegin;
    //     [mbegin..mend) - may be fully rewritten, selection will be kept
    //     [nend..mend) = ':' + whitespaces
    size_t nlen=nend-nbegin;


    //step 4 - pull and filter nicknames
    WStringVector nicks;
    {
        Roster::ContactListRef participants=rc->roster->getGroupContacts(roomGrp);

        for (Roster::ContactList::iterator i=participants->begin(); i!=participants->end(); i++) {
            std::wstring &ws=utf8::utf8_wchar( (*i)->jid.getResource() );

            if (ws.length()<clen) continue;
            if (clen>0) if (_wcsnicmp(buf+nbegin, ws.c_str(), clen)!=0) continue;

      if((*i)->status<5) nicks.push_back(ws);
        }
    }
    if (nicks.empty()) return;

    //step 5 - sorting
    stable_sort(nicks.begin(), nicks.end(), nickCompare);

    //step 6 - search for nick instance
    int loop=nicks.size();
    WStringVector::iterator i=nicks.begin();

    while (loop) {
        std::wstring &s=(*i);

        i++;
        if (i==nicks.end())
            i=nicks.begin();
        loop--;

        if (s.length()==nlen) {
            if (_wcsnicmp(buf+nbegin, s.c_str(), nlen)==0) break;
        }
    }


    std::wstring &s=(*i);
    s+=L": ";
    SendMessage(editWnd, EM_SETSEL, nbegin, mend);
    SendMessage(editWnd, EM_REPLACESEL, TRUE, (LPARAM)s.c_str());
    SendMessage(editWnd, EM_SETSEL, mbegin, nbegin+s.length());
}

void ChatView::setComposingState( bool composing ) {
    if (!Config::getInstance()->composing) return;
    if (!rc->isLoggedIn()) return;

    if (!contact->acceptComposing) return;
    if (composing==this->composing) return;

    this->composing=composing;

    std::string to=contact->jid.getJid();
    JabberDataBlockRef out=JabberDataBlockRef(new JabberDataBlock("message", NULL));
    out->setAttribute("to", to);
    out->addChildNS(
        (composing)? "composing" : "paused",
        "http://jabber.org/protocol/chatstates");

    /* xep-0022 deprecated; will be re-enabled with entity caps
    JabberDataBlockRef x=out->addChildNS("x", "jabber:x:event");
    x->addChild("id", NULL);
    if (composing) x->addChild("composing", NULL);
    */
    rc->jabberStream->sendStanza(*out);
}

//////////////////////////////////////////////////////////////////////////
class FontMetricCache {
public:
	FontMetricCache();
	~FontMetricCache();
	inline int getWidth(HDC hdc, wchar_t chr);
	int getHeight() {return Config::getInstance()->msg_font_height; };
private:
	LPINT page[256];
	int height;
};

FontMetricCache::FontMetricCache(){
	memset(page, 0, sizeof(page));
}

FontMetricCache::~FontMetricCache(){
	for (int i=0; i<256; i++) {
		if (page[i]) delete page[i];
	}
}

int FontMetricCache::getWidth(HDC hdc, wchar_t chr){
	int width_char = 0;
	GetCharWidth32(hdc, chr, chr, &width_char);
	return width_char;
	//return Config::getInstance()->msg_font_width;
}

FontMetricCache fmc;
 //Пользовательский шрифт чата,сделать соотв. опции

//////////////////////////////////////////////////////////////////////////
// WARNING!!! ONLY FOR WM2003 and higher
//////////////////////////////////////////////////////////////////////////
#ifndef DT_END_ELLIPSIS
#define DT_END_ELLIPSIS 0x00008000
#endif
//////////////////////////////////////////////////////////////////////////

void MessageElement::init() {
	//TODO: fix bug with cursor fit immediately after init();
	smiles=true; singleLine=false;
	RECT r={0,0,230,Config::getInstance()->msg_font_height}; //todo: fix width detection
	HDC tmp=CreateCompatibleDC(NULL);
	measure(tmp, r);
	DeleteDC(tmp);
	//height=10; width=10;
}

void MessageElement::measure(HDC hdc, RECT &rt) {
	//fmc.getWidth(hdc, ' ');
	if (width==rt.right-rt.left) {
		rt.bottom=rt.top+height; return;
	}//already measured уже свернуто

	//if (width>220) { //debug code
	//    SetBkMode(hdc, TRANSPARENT);
	//}
	render(hdc, rt, true);
	width=rt.right-rt.left;
	height=rt.bottom-rt.top;
}

void MessageElement::draw(HDC hdc, RECT &rt, bool znach) const {
	render(hdc, rt, false);
}

void MessageElement::render( HDC hdc, RECT &rt, bool measure ) const{
	HPEN hbr=NULL;
	int ypos=rt.top; //fmc.getHeight();

	const wchar_t *end=getText();
	const wchar_t *lineBegin=end;
	const wchar_t *wordBegin=NULL;
	const wchar_t *smileEnd;

	int smileIndex=-1;

	int xpos=rt.left;
	int xbegin=xpos;
	int mw=rt.right-GetSystemMetrics(SM_CXVSCROLL)/2;

	bool inUrl=FALSE;
	int lHeight=fmc.getHeight();

	if (!measure) if (singleLine) {
		skin->drawElement(hdc, icons::ICON_MSGCOLLAPSED_INDEX, xpos, ypos);
		xpos+=skin->getElementWidth()/2;
		xbegin=xpos;
	}

	strcpy((char*)FONT.lfFaceName, "Tahoma");
	FONT.lfHeight = Config::getInstance()->msg_font_height;//11-8 слишком толстый шрифт
	FONT.lfWidth = Config::getInstance()->msg_font_width;
	FONT.lfWeight = Config::getInstance()->tolshina;
	FONT.lfItalic = false;
	FONT.lfStrikeOut = false;
	FONT.lfUnderline = false;
	FONT.lfOrientation = 0;
	FONT.lfEscapement = 0;
	NormalFont  = CreateFontIndirectW(&FONT);
	SelectObject(hdc, NormalFont);


	wchar_t c;
	do {
		c=*end;
		switch (c) {
			case 0: break; //newline;
				//TODO: fix /n and /r/n
			case 0x0d: if (*(end+1)==0x0a) end++;
			case 0x0a: end++; if (!singleLine) break; //newline;

			case 0x01:
				if (hbr==NULL) {
					hbr=CreatePen(PS_SOLID, 0, GetTextColor(hdc));
					SelectObject(hdc, hbr);
				}
			case 0x02:
				if (!measure) ExtTextOut(hdc, xbegin, ypos, ETO_CLIPPED, &rt, lineBegin, end-lineBegin, NULL);
				if (inUrl) {
					//TODO: use underline font style instead of lines
					int h=ypos+fmc.getHeight()-1;
					MoveToEx(hdc,xbegin, h, NULL);
					LineTo(hdc, xpos, h);
				}
				inUrl=(c==0x1);
				end++;
				lineBegin=end; wordBegin=NULL; xbegin=xpos;
				continue;

				//some word delimiters
			case ' ':
			case '-':
			case '(':
			case ')':
			case ':':
			case '/':
			case '.':
			case ',':
				if (!inUrl) if (!singleLine) wordBegin=end+1;
			default:
				smileEnd=end;
				if (smiles) if (!inUrl) {
					smileIndex=smileParser->findSmile(&smileEnd);

					if (smileIndex>=0) {
						if (!measure) ExtTextOut(hdc, xbegin, ypos, ETO_CLIPPED, &rt, lineBegin, end-lineBegin, NULL);
						if (inUrl) {
							int h=ypos+fmc.getHeight()-1;
							MoveToEx(hdc,xbegin, h, NULL);
							LineTo(hdc, xpos, h);
						}
						int smileWidth=smileParser->icons->getElementWidth();
						lHeight=smileWidth;



						lineBegin=end=smileEnd; wordBegin=NULL;
						xbegin=xpos+smileWidth;
						if (xbegin>=mw) {ypos+=smileWidth;xpos=rt.left;xbegin=xpos+smileWidth;wordBegin=smileEnd;}

						if (!measure) {
							if (ypos<rt.bottom && ypos+smileWidth>=rt.top)
								smileParser->icons->drawElement(hdc,4*smileIndex+smile_anim_ind, xpos, ypos);
						}

						xpos=xbegin;


						continue;
					}
				}

				xpos+=fmc.getWidth(hdc, c);
				if (xpos<mw) {
					end++; continue;
				} else if (wordBegin) end=wordBegin;
		}

		const wchar_t *lineEnd=end;
		while (lineEnd>lineBegin) {
			if (*(lineEnd-1)>0x0d) break;
			lineEnd--;
		}

		if (!measure) ExtTextOut(hdc, xbegin, ypos, ETO_CLIPPED, &rt, lineBegin, lineEnd-lineBegin, NULL);
		if (inUrl) {
			int h=ypos+fmc.getHeight()-1;
			MoveToEx(hdc,xbegin, h, NULL);
			LineTo(hdc, xpos, h);
		}
		xbegin=rt.left;

		ypos+=lHeight; xpos=rt.left; lineBegin=end; wordBegin=NULL; //newline
		lHeight=fmc.getHeight();
		if (singleLine) break;
		//if (c) end++;
	} while (c);

	if (measure) rt.bottom=ypos;
	if (hbr) DeleteObject(hbr);
}

int MessageElement::getWidth() const { return width;}
int MessageElement::getHeight() const { return height; }


MessageElement::MessageElement(const std::string &str) {
	this->wstr=utf8::utf8_wchar(str);
	init();
}

const wchar_t * MessageElement::getText() const { return wstr.c_str(); }
int MessageElement::getColor() const { return 0; }







HMENU MessageElement::getContextMenu( HMENU menu ) {
	if (!menu)
		menu=CreatePopupMenu();
	else
		AppendMenu(menu, MF_SEPARATOR , 0, NULL);
        HMENU subMenuju=CreatePopupMenu();//подменю juick
		AppendMenu(subMenuju, MF_STRING, JUICK_NUM, L"#пост" );
		AppendMenu(subMenuju, MF_STRING, JUICK_COM, L"#пост/коммент." );
		AppendMenu(subMenuju, MF_STRING, JUICK_COM_K, L"пост+коммент." );
		AppendMenu(subMenuju, MF_STRING, JUICK_NUM_U, L"отписаться #пост" );

	if(mucak)AppendMenu(menu, MF_STRING , CGETNICK, L"Ответить" );
	AppendMenu(menu, MF_STRING, CQUOTE, L"Цитировать" );
	AppendMenu(menu, MF_SEPARATOR , 0, NULL);
	AppendMenu(menu, MF_STRING, WM_COPY, L"Копировать" );
	AppendMenu(menu, MF_STRING, GOTOURL, L"Открыть url" );
	AppendMenu(menu, MF_SEPARATOR , 0, NULL);
	AppendMenu(menu, MF_POPUP, (LPARAM)subMenuju, TEXT("juick"));
	AppendMenu(menu, MF_SEPARATOR , 0, NULL);
	AppendMenu(menu, (singleLine)? MF_STRING  :  MF_STRING | MF_CHECKED, IDOK, L"Свернуть" );
	AppendMenu(menu, (smiles)? MF_STRING | MF_CHECKED  :  MF_STRING, IDM_SMILES, L"Смайлы" );
	AppendMenu(menu, MF_SEPARATOR , 0, NULL);
	AppendMenu(menu, MF_STRING , CCLRMSGS, L"Очистить" );
	return menu;
}


bool MessageElement::OnMenuCommand(int cmdId, HWND parent, HWND edithwnd){

	switch (cmdId) {
        case WM_COPY:
            {
                std::wstring copy=wstr;
                // striping formating
                size_t i=0;
                while (i<copy.length()) {
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }
                int dsize=sizeof(wchar_t)*(copy.length()+1);
                HANDLE hmem=LocalAlloc(LPTR, dsize);
                if (!hmem) return true;
                memcpy(hmem, copy.c_str(), dsize);

                if (OpenClipboard(NULL)) {
                    EmptyClipboard(); //need to take ownership
                    SetClipboardData(CF_UNICODETEXT, hmem);
                    CloseClipboard();
                } else LocalFree(hmem);
                return true;
            }


     case GOTOURL:
		 {
			 DialogURL * dlgURL = new DialogURL(wstr);
			/*wchar_t *strurl=new wchar_t[wstr.length()+1];
				wcscpy(strurl,wstr.c_str());
				wchar_t *strurl2=new wchar_t[wstr.length()+1];


					strurl=wcsstr(strurl,L"http://");
			while(strurl!=NULL)
			{

				wcscpy(strurl2,strurl);
				strurl2=wcstok(strurl2,L" \n;\,<>*\"\'\[\]\{\}");
				strurl2[wcslen(strurl2)-1]='\0';
				int result=MessageBox(GetParent(edithwnd), strurl2, TEXT("Открыть URL?"), MB_YESNOCANCEL | MB_ICONWARNING );

			    	if (result==IDYES){
				ExecFile(strurl2,L"");
				delete[] strurl;
				delete[] strurl2;
				return true;}
					if (result==IDCANCEL){
				delete[] strurl;
				delete[] strurl2;
				return true;}

				memset(strurl,20,2);
 //result=MessageBox(NULL, strurl, TEXT("Открыть URL?1"), MB_YESNO | MB_ICONWARNING );

				strurl=wcsstr(strurl,L"http://");
//result=MessageBox(NULL, strurl, TEXT("Открыть URL?2"), MB_YESNO | MB_ICONWARNING );
			}
				delete[] strurl;
				delete[] strurl2;
                return true;*/
		 }
		return TRUE;
		 case JUICK_COM_K:
			 {std::wstring copy=wstr;
                // striping formating
                size_t i=0;
                while (i<copy.length()) {
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }
								if(copy.find(L"http://juick.com/")!=-1){

					copy.erase(0,copy.rfind(L"http://juick.com/"));
					copy.erase(0,copy.rfind('/')+1);
					i=0;
					while (i<copy.length()) {
                    if (copy[i]==' ') {
                        copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
                }
					i=0;
					while (i<copy.length()) {
						if (copy[i]=='#') {
							copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
					}
					copy.insert(0,L"#");
					copy+=L"+";
				//if(copy.rfind(' '))copy.erase(copy.rfind(' '));
					SendMessage(edithwnd, WM_SETTEXT, 1, (LPARAM)copy.c_str());
					SendMessage(edithwnd, JUICK_SEND, 0, 0);
								}
				SetFocus(edithwnd);
					  return true;
			 }
			 case JUICK_NUM_U:
			 {std::wstring copy=wstr;
                // striping formating
                size_t i=0;
                while (i<copy.length()) {
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }
								if(copy.find(L"http://juick.com/")!=-1){

					copy.erase(0,copy.rfind(L"http://juick.com/"));
					copy.erase(0,copy.rfind('/')+1);
					i=0;
					while (i<copy.length()) {
                    if (copy[i]==' ') {
                        copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
                }
					i=0;
					while (i<copy.length()) {
						if (copy[i]=='#') {
							copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
					}
					copy.insert(0,L"U #");
				//if(copy.rfind(' '))copy.erase(copy.rfind(' '));
					SendMessage(edithwnd, WM_SETTEXT, 1, (LPARAM)copy.c_str());}
				SetFocus(edithwnd);
					  return true;
			 }
			 case JUICK_NUM:
			 {std::wstring copy=wstr;
                // striping formating
                size_t i=0;
                while (i<copy.length()) {
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }
								if(copy.find(L"http://juick.com/")!=-1){

					copy.erase(0,copy.rfind(L"http://juick.com/"));
					copy.erase(0,copy.rfind('/')+1);
					i=0;
					while (i<copy.length()) {
                    if (copy[i]==' ') {
                        copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
                }
					i=0;
					while (i<copy.length()) {
						if (copy[i]=='#') {
							copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
					}
					copy.insert(0,L"#");
				//if(copy.rfind(' '))copy.erase(copy.rfind(' '));
					SendMessage(edithwnd, WM_SETTEXT, 1, (LPARAM)copy.c_str());}
				SetFocus(edithwnd);
					  return true;
			 }
		 case JUICK_COM:
			 {std::wstring copy=wstr;
                // striping formating
                size_t i=0;
                while (i<copy.length()) {
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }
				if(copy.find(L"http://juick.com/")!=-1){

					copy.erase(0,copy.rfind(L"http://juick.com/"));
					copy.erase(0,copy.rfind('/')+1);
					i=0;
					while (i<copy.length()) {
                    if (copy[i]==' ') {
                        copy.erase(i,copy.length());
                        continue;
                    }
                    i++;
                }
					i=0;
					while (i<copy.length()) {
						if (copy[i]=='#') {
							copy.replace(i,1,L"/");
                        continue;
                    }
                    i++;
					}
					copy.insert(0,L"#");
				//if(copy.rfind(' '))copy.erase(copy.rfind(' '));
					SendMessage(edithwnd, WM_SETTEXT, 1, (LPARAM)copy.c_str());}
				SetFocus(edithwnd);
					  return true;

			 }
		 case IDOK://это должно быть обязательно перед смайлами-они связаны
            {
                singleLine=!singleLine;
                smiles=TRUE;
				if(!singleLine){init();return true;}
                //
            }
         case IDM_SMILES:
            {
                smiles=!smiles;
				if(smiles){init();return true;}
                RECT rt={0,0,width,0};
                render(NULL, rt, true);
                width=rt.right-rt.left;
                height=rt.bottom-rt.top;
                InvalidateRect(parent, NULL, true);
                return true;
            }
		case CCLRMSGS:{
			PostMessage(parent, CLEARMESS, NULL, false);
			return true;}
		case CGETNICK:{
			if (!edithwnd) return true;

				std::wstring copy=wstr;

                // striping formating
                size_t i=0;bool okflag=0;
                while (i<copy.length()) {
					if(copy[i]=='>')okflag=1;
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }

				copy.erase(0,11);

				if(okflag){
					copy.replace(copy.find('>'),copy.length(),L": ");
				}
				else
				{	if(copy[0]=='*')copy.erase(0,1);
					copy.replace(copy.find(' '),copy.length(),L": ");
				}
				SendMessage(edithwnd, EM_REPLACESEL, TRUE, (LPARAM)copy.c_str());
				SetFocus(edithwnd);
					  return true;}
		case CQUOTE:
				if (!edithwnd) return true;
				//
				std::wstring copy=wstr;
                // striping formating
                size_t i=0;
                while (i<copy.length()) {
                    if (copy[i]<0x09) {
                        copy.erase(i,1);
                        continue;
                    }
                    i++;
                }
				copy=L"<<"+copy+L">> ";
				SendMessage(edithwnd, EM_REPLACESEL, TRUE, (LPARAM)copy.c_str());
				SetFocus(edithwnd);
				return true;
    }
    return false;
}
