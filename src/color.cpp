#include <string>
#include <windows.h>
#include <windowsx.h>
#include "utf8.hpp"

#include "Sysinfo.h"
extern std::wstring appRootPath;
/* int COLORS[]={
        0x000000,   // online
        0x39358b,   // chat
        0x008080,   // away
        0x535353,   // xa
        0x800000,   // dnd
        0x00,
        0x00,
        0x00,
		0xffffff,// focused item SetTextColor
		0x800000,//  bkColor=(GetFocus()==hWnd)?:;
        0x808080,
		0xffeeff,//bkColor= (oddIndex)?
		0xffffff
    }; 
*/
extern bool menu_font_is_m;
extern bool menu_font_is_s;
extern bool menu_font_is_p;
extern DWORD COLORS[];

int chartoint(char liter){
	if(liter>='a' && liter<='f')	return (liter+10-'a');
	if(liter>='0' && liter<='9')	return (liter-'0');
	return 0;
}
///char colorch[]="000000 39358b 008080 535353 800000 000000 000000 000000 ffffff 800000 808080 ffeeff ffffff";
int iii;

void colorsload(std::wstring txtname){
	 menu_font_is_m=1;
	menu_font_is_s=1;
	menu_font_is_p=1;
	std::wstring pathc=appRootPath;
	pathc+=L"color\\"+txtname;

    HANDLE filec=CreateFile(pathc.c_str(), 
        GENERIC_READ , 
        FILE_SHARE_READ, NULL, 
        OPEN_EXISTING ,
        0, NULL);

    if (filec==INVALID_HANDLE_VALUE) return;
    int size=GetFileSize(filec, NULL);
    if (size>64000) {CloseHandle(filec); return; }

    char *colorch=new char[size+1];
	DWORD rd;
    ReadFile(filec, colorch, size, &rd, NULL);     
    CloseHandle(filec);

    colorch[rd]=NULL;

	for ( int fff=0 ; fff<=24 ; fff++ ){
		iii=fff*7;
		COLORS[fff]=1048576*chartoint(colorch[iii])+65536*chartoint(colorch[iii+1])+4096*chartoint(colorch[iii+2])+256*chartoint(colorch[iii+3])+16*chartoint(colorch[iii+4])+chartoint(colorch[iii+5]);
//	printf("COLORS[%d] #%06X \n",fff,COLORS[fff]);
	}
}