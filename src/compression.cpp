
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <cfloat>

#include "ImageBase.h"

double PSNR( ImageBase &imOriginal,  ImageBase &imReconstructed) {

    double eqm = 0.0;
    int height = imOriginal.getHeight();
    int width = imOriginal.getWidth();
    int totalPixels = height * width;

    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            int rOriginal = imOriginal[x * 3][y * 3 + 0];
            int gOriginal = imOriginal[x * 3][y * 3 + 1];
            int bOriginal = imOriginal[x * 3][y * 3 + 2];

            int rReconstructed = imReconstructed[x * 3][y * 3 + 0];
            int gReconstructed = imReconstructed[x * 3][y * 3 + 1];
            int bReconstructed = imReconstructed[x * 3][y * 3 + 2];

            eqm += pow(rOriginal - rReconstructed, 2);
            eqm += pow(gOriginal - gReconstructed, 2);
            eqm += pow(bOriginal - bReconstructed, 2);
        }
    }

    eqm /= (totalPixels * 3); 

    if (eqm == 0) {
        return std::numeric_limits<double>::infinity(); 
    }

    double maxPixelValue = 255.0;
    double psnr = 10 * log10((maxPixelValue * maxPixelValue) / eqm);
    return psnr;
}

int main(int argc, char** argv) {


    if (argc != 3) {
        printf("Usage: ./compression imBase.ppm imSuperpixel.ppm\n");
        return 1;
    }

    std::string cnameImgBase = argv[1];
    std::string cnameImgSuperPixel = argv[2];
  
    std::string cNameImgIn = "images/" + cnameImgBase;
    std::string cNameImgOut = "output/" + cnameImgSuperPixel;
  
    char nameImgBase[250];
    char nameImgSuperPixel[250];

    std::strcpy(nameImgBase, cNameImgIn.c_str());
    std::strcpy(nameImgSuperPixel, cNameImgOut.c_str());


    ImageBase imBase;
    imBase.load(nameImgBase);
    ImageBase imSuperPixel; 
    imSuperPixel.load(nameImgSuperPixel);



    double psnr=PSNR(imBase,imSuperPixel);
    std::cout << "PSNR: " << psnr << " dB" << std::endl;

    return 0;
}