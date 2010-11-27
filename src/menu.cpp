#include "menu.h"
#include <string.h>
#include <windowsx.h>
#include <afxwin.h>
#include "Image.h"
#include "config.h"
extern DWORD COLORS[];
extern ImgListRef ImgMenu;
extern ImgListRef skin;
extern bool menu_font_is_s;
extern bool menu_font_is_m;
extern bool menu_font_is_f;
extern bool menu_font_is_p;
#define DEBUG 0

#if DEBUG == 1
#include <iostream>
#endif

#define STRLEN(s) wcslen(s)
#define STRCPY(d,s) wcscpy(d,s);

int Menu::MenuItem::_itemids = 10000;


MenuElement::MenuElement(Menu* rootmenu)
{
    _text = NULL;
    _icon = 0;
    _root = rootmenu;
	_separator=0;
}

MenuElement::MenuElement(Menu* rootmenu, const character_t* text, int icon)
{
    _text = NULL;
    _root = rootmenu;
    setText(text);
    setIcon(icon);
	_separator=0;
}

void MenuElement::setIcon(int icon)
{
    _icon = icon;
}

void MenuElement::setText(const character_t* text)
{
    if(_text) delete[] _text;
    _text = new character_t[STRLEN(text) + 1];
    STRCPY(_text,text);
}

void MenuElement::ownerdraw(DRAWITEMSTRUCT* di)
{
    #define DELTA_ICON_LEFT 2
    #define DELTA_ICON_TOP 1
    #define DELTA_TEXT_LEFT 9
	#define C__HIGHLIGHT COLORS[19]// цвет фона выделенный пункт RGB(255,255,100)
	#define C__HIGHLIGHTTEXT COLORS[20]// цвет текст выделенный пункт RGB(0xFF,0x10,0x10)
	#define C__MENU COLORS[21]//цвет фона RGB(200,200,200)
	#define C__MENUTEXT COLORS[22]//цвет текста RGB(155,100,255)
	#define C__GRAYTEXT COLORS[23]// отключенное меню RGB(0,0,0)
	#define C__SEPARATOR COLORS[24]// цвет сепаратора RGB(0,200,0)
static LOGFONTW FONT_M;    //
static HFONT NormalFont_M;
static HPEN hNewPen ;
static HBRUSH NewBrush_1;
static HBRUSH NewBrush_2;
  
    SIZE si;
    getIconSize(&si);
	
		
	
    if (di->itemState & ODS_SELECTED == ODS_SELECTED){
		if(menu_font_is_s) {
			menu_font_is_s=0;
			NewBrush_1 = CreateSolidBrush(C__HIGHLIGHT);
		}
    	HBRUSH OldBrush = (HBRUSH)SelectObject(di->hDC,NewBrush_1);

        FillRect(di->hDC, &di->rcItem, NewBrush_1/*GetSysColorBrush(COLOR_HIGHLIGHT)*/);
        SetBkMode(di->hDC, TRANSPARENT);
        SetBkColor(di->hDC, C__HIGHLIGHT/*GetSysColorBrush(COLOR_HIGHLIGHT)*/);
        SetTextColor(di->hDC,C__HIGHLIGHTTEXT /* GetSysColor(COLOR_HIGHLIGHTTEXT)*/);
        SelectObject(di->hDC,OldBrush);
	    //DeleteObject(NewBrush);
    }else{
		if(menu_font_is_m) {
			menu_font_is_m=0;
		NewBrush_2 = CreateSolidBrush(C__MENU);}
    	HBRUSH OldBrush = (HBRUSH)SelectObject(di->hDC,NewBrush_2);
        FillRect(di->hDC, &di->rcItem,NewBrush_2 /* GetSysColorBrush(COLOR_MENU)*/);
		SetBkMode(di->hDC, TRANSPARENT);
		SetBkColor(di->hDC,C__MENU);
        SetTextColor(di->hDC, C__MENUTEXT /*GetSysColor(COLOR_MENUTEXT)*/);
		SelectObject(di->hDC,OldBrush);
	   // DeleteObject(NewBrush);
    }
    if (di->itemState & ODS_GRAYED) {
        SetBkMode(di->hDC, TRANSPARENT);
        SetTextColor(di->hDC,C__GRAYTEXT /* GetSysColor(COLOR_GRAYTEXT)*/);
    }
    if(_icon != 0){
        
		if(_icon < 0x100){
			ImgMenu->drawElement(di->hDC, _icon,di->rcItem.left+DELTA_ICON_LEFT,di->rcItem.top+ DELTA_ICON_TOP);
		}else{
			skin->drawElement(di->hDC, (_icon-0x100),di->rcItem.left+DELTA_ICON_LEFT,di->rcItem.top+ DELTA_ICON_TOP);
		}
    }
    di->rcItem.left = di->rcItem.left + si.cx+ DELTA_ICON_LEFT ;
    SetBkMode(di->hDC, TRANSPARENT);
	if(_separator== 0){

		if (menu_font_is_f){
			menu_font_is_f=0;
			strcpy((char*)FONT_M.lfFaceName, "Tahoma"); 
	FONT_M.lfHeight = Config::getInstance()->menu_Height ;//Config::getInstance()->msg_font_height; 
	FONT_M.lfWidth = Config::getInstance()->menu_Width ;// Config::getInstance()->msg_font_width;
	FONT_M.lfWeight = Config::getInstance()->menu_Weight;//Config::getInstance()->tolshina;
			FONT_M.lfItalic = false; 
			FONT_M.lfStrikeOut = false; 
			FONT_M.lfUnderline = false; 
			FONT_M.lfOrientation = 0; 
			FONT_M.lfEscapement = 0; 
			NormalFont_M  = CreateFontIndirectW(&FONT_M); }
			HFONT oldfont=(HFONT)SelectObject(di->hDC, NormalFont_M);
	DrawText(di->hDC, _text, -1, &di->rcItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
			SelectObject(di->hDC, oldfont);
		//	DeleteObject(NormalFont_M);
	}
	else{
		if(menu_font_is_p) {menu_font_is_p=0;
		hNewPen = ::CreatePen(PS_SOLID,3,C__SEPARATOR);}
		HPEN hOldPen = (HPEN)SelectObject(di->hDC,hNewPen);
			MoveToEx(di->hDC,di->rcItem.right-1,di->rcItem.top+3,NULL);
			LineTo(di->hDC,di->rcItem.left,di->rcItem.top+3);
			SelectObject(di->hDC,hOldPen);
			//DeleteObject(hNewPen);
	}
//	if(menu_font_is_c)menu_font_is_c=0;
}

void MenuElement::measureitem(HDC dc, MEASUREITEMSTRUCT* mi)
{
    SIZE s;
    SIZE si;
    getIconSize(&si);
    if(_text != NULL){
        GetTextExtentPoint32(dc,_text,STRLEN(_text),&s);
    }
    mi->itemWidth = s.cx + si.cx+5;
	if(_separator== 0){mi->itemHeight= (s.cy>si.cy)?s.cy+2:si.cy+2;
	}else{mi->itemHeight=6;}
    #if DEBUG == 1
    std::cout<<s.cx<<","<<s.cy<<std::endl;
    #endif
}

void MenuElement::getIconSize(SIZE* s)
{
    if(_icon == 0){
        s->cx = 0;
        s->cy = 0;
        return;
    }
    ICONINFO inf;
   // GetIconInfo(_icon,&inf);
 //   BITMAP bmp;
   // GetObject(_icon,sizeof(BITMAP),&bmp);
    s->cx = ImgMenu->getElementWidth();//bmp.bmWidth;
    s->cy = ImgMenu->getElementHeight();//bmp.bmHeight;
}

Menu::Menu(HWND hWnd)
    :MenuElement(this)
{
    _submenus = new SubMenus;
    _subitems = new MenuItems;

    _hwnd = hWnd;

    _hmenu = CreateMenu();
}

Menu::Menu(Menu* rootmenu, const character_t* text, int icon)
    :MenuElement(rootmenu,text,icon)
{
    _submenus = new SubMenus;
    _subitems = new MenuItems;

    _hwnd = 0;

    _hmenu = CreatePopupMenu();
}

Menu::Menu(HWND hWnd,const character_t* text, int icon)
    :MenuElement(this,text,icon)
{
    _submenus = new SubMenus;
    _subitems = new MenuItems;

    _hwnd = hWnd;

    _hmenu = CreatePopupMenu();
}

Menu::~Menu()
{
    if(_submenus != NULL){
        for(SubMenus::iterator it = _submenus->begin(); it != _submenus->end(); it++){
            delete (*it);
        }
        delete _submenus;
    }
    
    if(_subitems != NULL){
        for(MenuItems::iterator it = _subitems->begin(); it != _subitems->end(); it++){
            delete (*it);
        }
        delete _subitems;
    }
}


HMENU Menu::hMenu()
{
    return _hmenu;
}

void Menu::set()
{
    if(_hwnd != 0 && _hmenu != 0){
        SetMenu(_hwnd,_hmenu);
    }
}

bool Menu::call(MenuItem::itemid_t itemid)
{
    if(_subitems != NULL){
        for(MenuItems::iterator it = _subitems->begin(); it != _subitems->end(); it++){
            if((*it)->call(itemid))return true;
        }
    }
    if(_submenus != NULL){
        for(SubMenus::iterator it = _submenus->begin(); it != _submenus->end(); it++){
            if((*it)->call(itemid))return true;
        }
    }
    return false;
}


void Menu::measure(MEASUREITEMSTRUCT* mi)
{
    if(mi->CtlType != ODT_MENU) return;
    MenuElement* element = reinterpret_cast<MenuElement*>(mi->itemData);
    if(element != NULL) element->measureitem(GetDC(_hwnd),mi);
}

void Menu::redraw(DRAWITEMSTRUCT* di)
{
    if(di->CtlType != ODT_MENU) return;
    MenuElement* element = reinterpret_cast<MenuElement*>(di->itemData);
    if(element != NULL) element->ownerdraw(di);
}


Menu* Menu::addMenu(const character_t* text, int icon)
{
    Menu* m = new Menu(_root,text, icon);
    _submenus->push_back(m);
    AppendMenu(_hmenu, MF_STRING | MF_POPUP | MF_OWNERDRAW, reinterpret_cast<UINT_PTR>(m->_hmenu), reinterpret_cast<character_t*>(m));//m->_text
    return m;
}

void Menu::addSeparator()
{
    MenuItem* item = new MenuItem(_root, L"Separator", 0, NULL);
	item->_separator=true;
   _subitems->push_back(item);
    AppendMenu(_hmenu,  MF_SEPARATOR| MF_OWNERDRAW, item->_id, reinterpret_cast<LPCWSTR>(item));
  
}

Menu::MenuItem* Menu::addItem(const character_t* text, int icon, MenuItem::Handler proc)
{
    MenuItem* item = new MenuItem(_root, text, icon, proc);
    _subitems->push_back(item);
    AppendMenu(_hmenu, MF_OWNERDRAW, item->_id, reinterpret_cast<LPCWSTR>(item));
    return item;
}

Menu::MenuItem* Menu::addItem(const character_t* text, int icon, MenuItem::Handler proc, MenuItem::itemid_t itemid)
{
    MenuItem* item = new MenuItem(_root, text, icon, proc, itemid);
    _subitems->push_back(item);
    AppendMenu(_hmenu, MF_OWNERDRAW, item->_id, reinterpret_cast<LPCWSTR>(item));
    return item;
}
//MenuItem

Menu::MenuItem::MenuItem(Menu* rootmenu, itemid_t itemid)
    :MenuElement(rootmenu)
{
    _handler = NULL;
    _id = itemid;
}

Menu::MenuItem::MenuItem(Menu* rootmenu, const character_t* text, int icon, Handler proc)
    :MenuElement(rootmenu,text,icon)
{
    _handler = proc;
    _id = _itemids ++;
}

Menu::MenuItem::MenuItem(Menu* rootmenu, const character_t* text, int icon, Handler proc, itemid_t itemid)
    :MenuElement(rootmenu,text,icon)
{
    _handler = proc;
    _id = itemid;
}

int Menu::MenuItem::id()
{
    return _id;
}

void Menu::MenuItem::setHandler(Handler proc)
{
    _handler = proc;
}

bool Menu::MenuItem::call(itemid_t itemid)
{
    if(itemid == _id){
        if(_handler != NULL) _handler();
        return true;
    }
    return false;
}

void Menu::MenuItem::check(bool ch)
{
    CheckMenuItem(_root->_hmenu,_id,MF_BYCOMMAND | ch?MF_CHECKED:MF_UNCHECKED);
}

bool Menu::MenuItem::checked()
{
    return GetMenuState(_root->_hmenu,_id,MF_BYCOMMAND) & MF_CHECKED;
}
