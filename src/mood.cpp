#include "mood.h"
#include <string>
extern std::wstring skinRootPath;
extern char ***getConfig(const wchar_t *fileName,int *count);

moodParse::moodParse(){
    loadMood();
}

moodParse::~moodParse(){
};



void moodParse::loadMood(){
iconsMood=ImgListRef(new ImgArray(L"moods.png", 16, -1));
if (!iconsMood) return;
std::wstring moodpatch=skinRootPath+TEXT("moods.txt");
lines=getConfig(moodpatch.c_str(),&linesCount);

}