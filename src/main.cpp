#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>

#include "ImageBase.h"

int main(int argc, char **argv) {
  if (argc != 2) {
		printf("Usage: ./main vader.ppm \n"); 
		return 1;
	}

  std::string cNameImg = argv[1];
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

	ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
	for(int x = 0; x < imIn.getHeight(); ++x) {
		for(int y = 0; y < imIn.getWidth(); ++y) {
			imOut[y*3][x*3+0] = imgR[y][x];
			imOut[y*3][x*3+1] = imgG[y][x];
			imOut[y*3][x*3+2] = imgB[y][x];
		}
	}
	imOut.save(nameImgOur);
  
	return 0;
}
