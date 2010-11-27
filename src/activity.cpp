#include "activity.h"
#include <string>
extern std::wstring skinRootPath;
extern char ***getConfig(const wchar_t *fileName,int *count);

actParse::actParse(){
    loadact();
}

actParse::~actParse(){
};



void actParse::loadact(){
iconsact=ImgListRef(new ImgArray(L"activ.png", 16, -1));
if (!iconsact) return;
std::wstring actpatch=skinRootPath+TEXT("activity.txt");
lines=getConfig(actpatch.c_str(),&linesCount);

}