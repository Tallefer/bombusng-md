#pragma once

#include <string>
#include "Image.h"
class moodParse{
public:
    moodParse();
    ~moodParse();

   
    ImgListRef iconsMood;

 int linesCount;
	char ***lines;//lines[i][1]-русский lines[vybor_moods2][0]-в потоке

   

protected:
    void loadMood();
  }; 
