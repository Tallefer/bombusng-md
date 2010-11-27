#pragma once

#include <string>
#include "Image.h"
class actParse{
public:
    actParse();
    ~actParse();

    
    ImgListRef iconsact;

 int linesCount;
	char ***lines;//lines[i][1]-русский lines[vybor_moods2][0]-в потоке

   

protected:
    void loadact();
  }; 