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
#include "Vec4.h"

const int COMPACTNESS = 10; // m
std::map<int, Vec3> clusterColors;

void averageClusterColor(ImageBase& imIn, Vec2 center) {
  // TODO: Naive approach. Must be changed
  ImageBase imR = *imIn.getPlan(ImageBase::PLAN_R);
  ImageBase imG = *imIn.getPlan(ImageBase::PLAN_G);
  ImageBase imB = *imIn.getPlan(ImageBase::PLAN_B);

  int x = center[0];
  int y = center[1];

  int sumR = 0;
  int countR = 0;
  int sumG = 0;
  int countG = 0;
  int sumB = 0;
  int countB = 0;
  for (int j = y - 1; j <= y + 1; j++) {
    for (int i = x - 1; i <= x + 1; i++) {
      sumR += imR[i][j];
      sumG += imG[i][j];
      sumB += imB[i][j];
      countR++;
      countG++;
      countB++;
    }
  }

  Vec3 color(sumR/countR, sumG/countG, sumB/countB);

  int centerId = ((x + y) * (x + y + 1)) / 2 + y;
  clusterColors[centerId] = color;
}

std::vector<Vec2> findInitialCenters(ImageBase& imIn, int S) {
  std::vector<Vec2> centers;
  for (int y = 0; y < imIn.getHeight(); ++y) {
    for (int x = 0; x < imIn.getWidth(); ++x) {
      if (x % S == S / 2 && y % S == S / 2) {
        centers.push_back(Vec2(x, y));
        averageClusterColor(imIn, Vec2(x, y));
        // TODO: Adjust centers in 3x3 neighborhood
      }
    }
  }
  return centers;
}

float colorDistance(ImageBase& imIn, Vec2 pixel, Vec2 center) {
  return 10;

  // This is too slow
  ImageBase imR = *imIn.getPlan(ImageBase::PLAN_R);
  ImageBase imG = *imIn.getPlan(ImageBase::PLAN_G);
  ImageBase imB = *imIn.getPlan(ImageBase::PLAN_B);

  int pixelX = pixel[0];
  int pixelY = pixel[1];
  int centerX = center[0];
  int centerY = center[1];
  Vec3 pixelColor(imR[pixelX][pixelY], imG[pixelX][pixelY], imB[pixelX][pixelY]);
  Vec3 centerColor(imR[centerX][centerY], imG[centerX][centerY], imB[centerX][centerY]);

  Vec3 colorDistance = pixelColor - centerColor;
  return colorDistance.length();
}

void createClusters(ImageBase& imIn, std::vector<Vec2> centers, int S) {
  for (int y = 0; y < imIn.getHeight(); ++y) {
    for (int x = 0; x < imIn.getWidth(); ++x) {
      Vec2 pixel(x, y);
      float bestDistance = 100.0;
      int bestCenterIdx;
      Vec3 bestCenterColor;
      for (int i = 0; i < centers.size(); i++) { // TODO:Optimize
        Vec2 center = centers[i];
        float distanceXY = (center - pixel).length();
        if (distanceXY > 2*S) continue;
        float distanceC = colorDistance(imIn, pixel, center);

        float distance = distanceC + (COMPACTNESS / S) * distanceXY;

        if (distance < bestDistance) {
          bestDistance = distance;
          bestCenterIdx = i;
          
          int a = center[0];
          int b = center[1];
          int centerId = ((a + b) * (a + b + 1)) / 2 + b;
          bestCenterColor = clusterColors[centerId];
        }
      }
      // This part must be changed
      // ___________________ 
      imIn[x * 3][y * 3 + 0] = bestCenterColor[0];
      imIn[x * 3][y * 3 + 1] = bestCenterColor[1];
      imIn[x * 3][y * 3 + 2] = bestCenterColor[2];
      // ___________________
    }
  }
}

void drawCenters(ImageBase& imIn, std::vector<Vec2> centers) {
  for (Vec2 center : centers) {
    int x = center[0];
    int y = center[1];

    for (int i = y - 1; i <= y + 1; i++) {
      for (int j = x - 1; j <= x + 1; j++) {
        imIn[j * 3][i * 3 + 0] = 128;
        imIn[j * 3][i * 3 + 1] = 255;
        imIn[j * 3][i * 3 + 2] = 128;
      }
    }
  }
}

int main(int argc, char** argv) {
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

  int S = std::sqrt((imIn.getHeight() * imIn.getWidth()) / nSuperpixels); // Distance between centers
  std::vector<Vec2> centers = findInitialCenters(imIn, S);

  ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
  for (int y = 0; y < imIn.getHeight(); ++y) {
    for (int x = 0; x < imIn.getWidth(); ++x) {
      imOut[x * 3][y * 3 + 0] = imIn[x * 3][y * 3 + 0];
      imOut[x * 3][y * 3 + 1] = imIn[x * 3][y * 3 + 1];
      imOut[x * 3][y * 3 + 2] = imIn[x * 3][y * 3 + 2];
    }
  }
  createClusters(imOut, centers, S);
  drawCenters(imOut, centers);
  imOut.save(nameImgOur);

  return 0;
}
