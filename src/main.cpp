#include <stdio.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "ImageBase.h"

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: ./main vader.ppm 128\n");
    return 1;
  }

  int nSuperpixels;
  std::string cNameImg = argv[1];
  sscanf(argv[2], "%d", &nSuperpixels);

  std::string cNameImgIn = "images/" + cNameImg;
  std::string cNameImgOut = "output/" + cNameImg;

  char nameImgIn[250];
  char nameImgOur[250];
  std::strcpy(nameImgIn, cNameImgIn.c_str());
  std::strcpy(nameImgOur, cNameImgOut.c_str());

  ImageBase imIn;
  imIn.load(nameImgIn);

  ImageBase imgR = *imIn.getPlan(ImageBase::PLAN_R);
  ImageBase imgG = *imIn.getPlan(ImageBase::PLAN_G);
  ImageBase imgB = *imIn.getPlan(ImageBase::PLAN_B);

  int S = std::sqrt((imIn.getHeight() * imIn.getWidth()) / nSuperpixels);
  ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
  for (int x = 0; x < imIn.getHeight(); ++x) {
    for (int y = 0; y < imIn.getWidth(); ++y) {
      if (x % S == 0 || y % S == 0) {
        imOut[y * 3][x * 3 + 0] = 255;
        imOut[y * 3][x * 3 + 1] = 255;
        imOut[y * 3][x * 3 + 2] = 0;
      } else if (x % S == S/2 && y % S == S/2) {
        imOut[y * 3][x * 3 + 0] = 255;
        imOut[y * 3][x * 3 + 1] = 0;
        imOut[y * 3][x * 3 + 2] = 0;
      } else {
        imOut[y * 3][x * 3 + 0] = imgR[y][x];
        imOut[y * 3][x * 3 + 1] = imgG[y][x];
        imOut[y * 3][x * 3 + 2] = imgB[y][x];
      }
    }
  }
  imOut.save(nameImgOur);

  return 0;
}
