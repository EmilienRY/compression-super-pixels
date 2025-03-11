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

const int COMPACTNESS = 10; // m

std::vector<Vec3> clusterColors;
std::vector<Vec2> clusterCenters;



// convert RGB color to CIELAB color. 
// Do it by firt conveting to XYZ color space.
  Vec3 RGBtoCIELAB(Vec3 color) { 

    Vec3 RGB;
    for (int i = 0; i < 3; i++) {
        float temp = color[i] / 255.0f;
        if (temp > 0.04045f) {
            temp = powf((temp + 0.055f) / 1.055f, 2.4f);
        } else {
            temp /= 12.92f;
        }
        RGB[i] = temp * 100.0f;
    }

    float X = RGB[0] * 0.4124f + RGB[1] * 0.3576f + RGB[2] * 0.1805f;
    float Y = RGB[0] * 0.2126f + RGB[1] * 0.7152f + RGB[2] * 0.0722f;
    float Z = RGB[0] * 0.0193f + RGB[1] * 0.1192f + RGB[2] * 0.9505f;

    X /= 95.047f;
    Y /= 100.0f;
    Z /= 108.883f;

    Vec3 XYZ;
    XYZ[0] = (X > 0.008856f) ? powf(X, 1.0f / 3.0f) : (7.787f * X) + (16.0f / 116.0f);
    XYZ[1] = (Y > 0.008856f) ? powf(Y, 1.0f / 3.0f) : (7.787f * Y) + (16.0f / 116.0f);
    XYZ[2] = (Z > 0.008856f) ? powf(Z, 1.0f / 3.0f) : (7.787f * Z) + (16.0f / 116.0f);

    Vec3 LAB;
    LAB[0] = (116.0f * XYZ[1]) - 16.0f;
    LAB[1] = 500.0f * (XYZ[0] - XYZ[1]);
    LAB[2] = 200.0f * (XYZ[1] - XYZ[2]);

    return LAB;
}




Vec3 averageClusterColor(ImageBase& imIn, Vec2 center) {
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
      if(i<imR.getHeight() && i>=0 && j<imR.getWidth() && j>=0){
        sumR += imR[i][j];
        sumG += imG[i][j];
        sumB += imB[i][j];
        countR++;
        countG++;
        countB++;
      }

    }
  }

  Vec3 color(sumR/countR, sumG/countG, sumB/countB);
  return color;
}

void findInitialCenters(ImageBase& imIn, int S) {
  for (int y = 0; y < imIn.getWidth(); ++y) {
    for (int x = 0; x < imIn.getHeight(); ++x) {
      if (x % S == S / 2 && y % S == S / 2) {
        clusterCenters.push_back(Vec2(x, y));
        clusterColors.push_back(averageClusterColor(imIn, Vec2(x, y)));
        // TODO: Adjust centers in 3x3 neighborhood
      }
    }
  }
}

float colorDistance(ImageBase& imIn, Vec2 pixel, Vec2 center) {
  //TODO: This kinda works but should get the LAB distance instead

  int pixelX = pixel[0];
  int pixelY = pixel[1];

  int r1 = imIn[pixelX * 3][pixelY * 3 + 0];
  int g1 = imIn[pixelX * 3][pixelY * 3 + 1];
  int b1 = imIn[pixelX * 3][pixelY * 3 + 2];

  int centerX = center[0];
  int centerY = center[1];
  int r2 = imIn[centerX * 3][centerY * 3 + 0];
  int g2 = imIn[centerX * 3][centerY * 3 + 1];
  int b2 = imIn[centerX * 3][centerY * 3 + 2];

  int dr = r1 - r2;
  int dg = g1 - g2;
  int db = b1 - b2;

  return dr * dr + dg * dg + db * db;
}

void createClusters(ImageBase& imIn, int S) {
  for (int y = 0; y < imIn.getWidth(); ++y) {
    for (int x = 0; x < imIn.getHeight(); ++x) {
      Vec2 pixel(x, y);
      float bestDistance = -1;
      int bestCenterIdx;
      for (int i = 0; i < clusterCenters.size(); i++) {
        Vec2 center = clusterCenters[i];
        if (std::abs(center[0] - x) > S || std::abs(center[1] - y) > S) {
          continue;
        } 
        
        float distanceXY = (center - pixel).length();
        float distanceC = colorDistance(imIn, pixel, center);
        float distance = distanceC + (COMPACTNESS / S) * distanceXY;

        if (distance < bestDistance || bestDistance == -1) {
          bestDistance = distance;
          bestCenterIdx = i;
        }
      }
      // This part must be changed
      // ___________________ 
      Vec3 clusterColor = clusterColors[bestCenterIdx];
      imIn[x * 3][y * 3 + 0] = clusterColor[0];
      imIn[x * 3][y * 3 + 1] = clusterColor[1];
      imIn[x * 3][y * 3 + 2] = clusterColor[2];
      // ___________________
    }
  }
}

void drawCenters(ImageBase& imIn) {
  for (int i = 0; i < clusterCenters.size(); i++) {
    Vec2 center = clusterCenters[i];
    int x = center[0];
    int y = center[1];

    for (int i = y - 1; i <= y + 1; i++) {
      for (int j = x - 1; j <= x + 1; j++) {
        imIn[j * 3][i * 3 + 0] = 128;
        imIn[j * 3][i * 3 + 1] = 255;
        imIn[j * 3][i * 3 + 2] = 128;
      }
    }
    imIn[x * 3][y * 3 + 0] = clusterColors[i][0];
    imIn[x * 3][y * 3 + 1] = clusterColors[i][1];
    imIn[x * 3][y * 3 + 2] = clusterColors[i][2];
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
  findInitialCenters(imIn, S);
  ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
  for (int y = 0; y < imIn.getWidth(); ++y) {
    for (int x = 0; x < imIn.getHeight(); ++x) {
      imOut[x * 3][y * 3 + 0] = imIn[x * 3][y * 3 + 0];
      imOut[x * 3][y * 3 + 1] = imIn[x * 3][y * 3 + 1];
      imOut[x * 3][y * 3 + 2] = imIn[x * 3][y * 3 + 2];
    }
  }
  createClusters(imOut, S);
  drawCenters(imOut);
  imOut.save(nameImgOur);

  return 0;
}
