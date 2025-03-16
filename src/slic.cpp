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

std::vector<Vec2> clusterCenters;
std::vector<Vec3> clusterLABs;

std::vector<Vec2> clusterCentersSums;
std::vector<Vec3> clusterLABsSums;
std::vector<int> clusterCounts;

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

Vec3 CIELABtoRGB(Vec3 color) { 
  double var_Y = (color[0] + 16.0) / 116.0;
  double var_X = color[1] / 500.0 + var_Y;
  double var_Z = var_Y - color[2] / 200.0;

  if (std::pow(var_Y, 3) > 0.008856) {
    var_Y = std::pow(var_Y, 3);
  } else {
    var_Y = (var_Y - 16.0 / 116.0) / 7.787;
  }

  if (std::pow(var_X, 3) > 0.008856) {
    var_X = std::pow(var_X, 3);
  } else {
    var_X = (var_X - 16.0 / 116.0) / 7.787;
  }

  if (std::pow(var_Z, 3) > 0.008856) {
    var_Z = std::pow(var_Z, 3);
  } else {
    var_Z = (var_Z - 16.0 / 116.0) / 7.787;
  }

  Vec3 XYZ;
  XYZ[0] = var_X * 95.047f / 100;
  XYZ[1] = var_Y * 100.0f / 100;
  XYZ[2] = var_Z * 108.883f / 100;

  double var_R = XYZ[0] * 3.2406 + XYZ[1] * -1.5372 + XYZ[2] * -0.4986;
  double var_G = XYZ[0] * -0.9689 + XYZ[1] * 1.8758 + XYZ[2] * 0.0415;
  double var_B = XYZ[0] * 0.0557 + XYZ[1] * -0.2040 + XYZ[2] * 1.0570;

  if (var_R > 0.0031308) {
    var_R = 1.055 * std::pow(var_R, 1.0 / 2.4) - 0.055;
  } else {
    var_R = 12.92 * var_R;
  }

  if (var_G > 0.0031308) {
    var_G = 1.055 * std::pow(var_G, 1.0 / 2.4) - 0.055;
  } else {
    var_G = 12.92 * var_G;
  }

  if (var_B > 0.0031308) {
    var_B = 1.055 * std::pow(var_B, 1.0 / 2.4) - 0.055;
  } else {
    var_B = 12.92 * var_B;
  }

  Vec3 RGB;
  RGB[0] = (int) (var_R * 255);
  RGB[1] = (int) (var_G * 255);
  RGB[2] = (int) (var_B * 255);
  return RGB;
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
        
        Vec3 rgbColor = averageClusterColor(imIn, Vec2(x, y));
        Vec3 labColor = RGBtoCIELAB(rgbColor);
        clusterLABs.push_back(labColor);

        clusterCentersSums.push_back(Vec2(0.0, 0.0));
        clusterLABsSums.push_back(Vec3(0.0, 0.0, 0.0));
        clusterCounts.push_back(0);

        // TODO: Adjust centers in 3x3 neighborhood
      }
    }
  }
}

Vec3 getPixelColor(ImageBase& imIn, Vec2 pixel) {
  int pixelX = pixel[0];
  int pixelY = pixel[1];
  int r = imIn[pixelX * 3][pixelY * 3 + 0];
  int g = imIn[pixelX * 3][pixelY * 3 + 1];
  int b = imIn[pixelX * 3][pixelY * 3 + 2];
  return Vec3(r, g, b);
}

float colorDistance(ImageBase& imIn, Vec2 pixel, Vec2 center) {
  Vec3 colorRGB1 = getPixelColor(imIn, pixel);
  Vec3 colorLAB1 = RGBtoCIELAB(colorRGB1);

  Vec3 colorRGB2 = getPixelColor(imIn, center);
  Vec3 colorLAB2 = RGBtoCIELAB(colorRGB2);

  float distance = (colorLAB2-colorLAB1).length();

  return distance;
}

int findClosestClusterCenter(ImageBase& imIn, Vec2 pixel, int S) {
  float bestDistance = -1;
  int bestCenterIdx;
  for (int i = 0; i < clusterCenters.size(); i++) {
    Vec2 center = clusterCenters[i];
    if (std::abs(center[0] - pixel[0]) > S || std::abs(center[1] - pixel[1]) > S) {
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
  return bestCenterIdx;
}

void createClusters(ImageBase& imIn, int S) {
  float averageDistance;
  int iterations = 0;
  do {
    for (int y = 0; y < imIn.getWidth(); ++y) {
      for (int x = 0; x < imIn.getHeight(); ++x) {
        Vec2 pixel(x, y);
        int closestClusterId = findClosestClusterCenter(imIn, pixel, S);

        Vec2 center = clusterCenters[closestClusterId];
        Vec3 clusterLAB = clusterLABs[closestClusterId];

        Vec3 pixelRGB = getPixelColor(imIn, pixel);
        Vec3 pixelLAB = RGBtoCIELAB(pixelRGB);

        clusterCentersSums[closestClusterId] += pixel;
        clusterLABsSums[closestClusterId] += pixelLAB;

        clusterCounts[closestClusterId]++;
        
        Vec3 colorRGB = CIELABtoRGB(clusterLAB);
        imIn[x * 3][y * 3 + 0] = colorRGB[0];
        imIn[x * 3][y * 3 + 1] = colorRGB[1];
        imIn[x * 3][y * 3 + 2] = colorRGB[2];
      }
    }

    int count = 0;
    float sum = 0;
    std::vector<Vec2> newClusterCenters;
    std::vector<Vec3> newClusterLABs;
    for (int c = 0; c < clusterCenters.size(); c++) {
      Vec2 averageXY = clusterCentersSums[c] / clusterCounts[c];
      Vec3 averageLAB = clusterLABsSums[c] / clusterCounts[c];
      newClusterCenters.push_back(averageXY);
      newClusterLABs.push_back(averageLAB);

      Vec2 center = clusterCenters[c];
      Vec3 lab = clusterLABs[c];


      float distanceXY = (center - averageXY).length();
      float distanceC =  (lab - averageLAB).length();
      float distance = distanceC + (COMPACTNESS / S) * distanceXY;

      sum += distance;
      count++;
    }
    averageDistance = sum / count;


    clusterCenters = newClusterCenters;
    clusterLABs = newClusterLABs;
    clusterCentersSums.clear();
    clusterCounts.clear();
    clusterLABsSums.clear();

    iterations++;
  } while (averageDistance > 0.1);
  std::cout << "Iterations to convergence: " << iterations << std::endl;
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
    imIn[x * 3][y * 3 + 0] = clusterLABs[i][0];
    imIn[x * 3][y * 3 + 1] = clusterLABs[i][1];
    imIn[x * 3][y * 3 + 2] = clusterLABs[i][2];
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
  //drawCenters(imOut);
  imOut.save(nameImgOur);

  return 0;
}
