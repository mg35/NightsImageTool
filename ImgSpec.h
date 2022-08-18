//
// Created by 35mil on 8/1/2021.
//

#ifndef TESTPROJ_IMGSPEC_H
#define TESTPROJ_IMGSPEC_

enum bitWidth {FOUR_BIT, EIGHT_BIT, SIXTEEN_BIT};

struct ImgSpec {
   int height = 0;
   int width = 0;
   bool smallColors = false;
   bitWidth pixelEncoding = EIGHT_BIT;
   int offset = 0;
};


#endif //TESTPROJ_IMGSPEC_H
