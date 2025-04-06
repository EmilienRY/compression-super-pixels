#include <stdio.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>

#include "ImageBase.h"
#include "Vec2.h"
#include "Vec3.h"

int main(int argc, char** argv) {

  
    int nSuperpixels;
    std::string cNameImgBase = argv[1];
    std::string cNameImgDecomp = argv[2];

  
    std::string cNameImgIn = "images/" + cNameImgBase;
    std::string cNameImgOut = "output/" + cNameImgDecomp;
  
    char nameImgIn[250];
    char nameImgOur[250];
    std::strcpy(nameImgIn, cNameImgIn.c_str());
    std::strcpy(nameImgOur, cNameImgOut.c_str());
  
    ImageBase imIn;
    imIn.load(nameImgIn);

    ImageBase imDecomp;
    imDecomp.load(nameImgOur);
  
  
    std::cout << ImageBase::PSNR(imIn, imDecomp) << std::endl;
  
    return 0;
  }