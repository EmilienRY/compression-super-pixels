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
#include "Vec2.h"
#include "Vec3.h"

using namespace std;

class ElemStruct{

    public:
    
        int taille;
        vector<vector<int>> modifCo;
    
        ElemStruct(void);
    
        ElemStruct(int tailleEleme,vector<vector<int>> elem){
            taille=tailleEleme;
            modifCo=elem;
        }
    
};

struct SuperPixel {
    vector<Vec2> pixels;  
    Vec3 meanColor;       
};


void gradients(ImageBase &imIn,ImageBase &normeGradient){
    for(int x=0;x<imIn.getHeight();x++){
        for(int y=0;y<imIn.getWidth();y++){	

            float hori=(y+1)<imIn.getWidth() ? imIn[x][y+1]-imIn[x][y] : 0 ;
            float ver=(x+1)<imIn.getHeight() ? imIn[x+1][y]-imIn[x][y] : 0;
            float norme=sqrt(pow(hori,2)+pow(ver,2));

            normeGradient[x][y]=clamp((int)(norme),0,255);;

        }
    }

}


void gradientSobel(ImageBase &imIn, ImageBase &gradient) {
    int sobelX[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
    int sobelY[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

    for (int x = 1; x < imIn.getHeight() - 1; x++) {
        for (int y = 1; y < imIn.getWidth() - 1; y++) {
            float gx = 0, gy = 0;
            for (int i = -1; i <= 1; i++) {
                for (int j = -1; j <= 1; j++) {
                    gx += imIn[x + i][y + j] * sobelX[i + 1][j + 1];
                    gy += imIn[x + i][y + j] * sobelY[i + 1][j + 1];
                }
            }
            gradient[x][y] = clamp((int)sqrt(gx * gx + gy * gy), 0, 255);
        }
    }
}



void filtreMoy(ImageBase &imIn,ImageBase &imFlou,ElemStruct &e){

    for(int x=0;x<imIn.getHeight();x++){
        for(int y=0;y<imIn.getWidth();y++){	
            int cpt=0;
            int temp=0;
            for(int i=0;i<e.taille;i++){
                int newX = x + e.modifCo[i][0];
                int newY = y + e.modifCo[i][1];

                if(newX >= 0 && newX < imIn.getHeight() && newY >= 0 && newY < imIn.getWidth()){
                    cpt+=1;
                    temp+=imIn[newX][newY];
                }

            }

            imFlou[x][y]=(double)temp/(double)cpt;

        }
    }
}


void filtreGaussien(ImageBase &imIn, ImageBase &imOut, int rayon) {
    vector<vector<double>> kernel(2 * rayon + 1, vector<double>(2 * rayon + 1));
    double sigma = rayon / 2.0;
    double somme = 0.0;

    for (int i = -rayon; i <= rayon; i++) {
        for (int j = -rayon; j <= rayon; j++) {
            kernel[i + rayon][j + rayon] = exp(-(i * i + j * j) / (2 * sigma * sigma));
            somme += kernel[i + rayon][j + rayon];
        }
    }

    for (int i = 0; i < imIn.getHeight(); i++) {
        for (int j = 0; j < imIn.getWidth(); j++) {
            double sum = 0;
            double weightSum = 0;
            for (int u = -rayon; u <= rayon; u++) {
                for (int v = -rayon; v <= rayon; v++) {
                    int x = clamp(i + u, 0, imIn.getHeight() - 1);
                    int y = clamp(j + v, 0, imIn.getWidth() - 1);
                    sum += imIn[x][y] * kernel[u + rayon][v + rayon];
                    weightSum += kernel[u + rayon][v + rayon];
                }
            }
            imOut[i][j] = sum / weightSum;
        }
    }
}




void convertPPMtoPGM(ImageBase &imIn, ImageBase &Y) {
    for (int x = 0; x < imIn.getHeight(); ++x) {
        for (int y = 0; y < imIn.getWidth(); ++y) {
            Y[x][y] = 0.3 * imIn[x * 3][y * 3] + 
                      0.6 * imIn[x * 3][y * 3 + 1] + 
                      0.1 * imIn[x * 3][y * 3 + 2];       
        }
    }
}



vector<Vec2> findMarkers(ImageBase &gradient, int gradientThreshold) {
    vector<Vec2> markers;
    int width = gradient.getWidth();
    int height = gradient.getHeight();
    vector<vector<bool>> visited(height, vector<bool>(width, false));

    for (int x = 1; x < height - 1; x++) {
        for (int y = 1; y < width - 1; y++) {
            if (visited[x][y]) continue;

            int g = gradient[x][y];

            if (g < gradient[x - 1][y] && g < gradient[x + 1][y] &&
                g < gradient[x][y - 1] && g < gradient[x][y + 1]) {
                
                queue<Vec2> q;
                q.push(Vec2(x, y));
                visited[x][y] = true;

                int sumX = x, sumY = y, count = 1;

                while (!q.empty()) {
                    Vec2 current = q.front();
                    q.pop();

                    for (Vec2 dir : {Vec2(0, 1), Vec2(0, -1), Vec2(1, 0), Vec2(-1, 0)}) {
                        int nx = current[0] + dir[0];
                        int ny = current[1] + dir[1];

                        if (nx >= 0 && nx < height && ny >= 0 && ny < width && !visited[nx][ny]) {
                            int ng = gradient[nx][ny];
                            if (abs(ng - g) < gradientThreshold) { 
                                visited[nx][ny] = true;
                                q.push(Vec2(nx, ny));
                                sumX += nx;
                                sumY += ny;
                                count++;
                            }
                        }
                    }
                }

                markers.push_back(Vec2(sumX / count, sumY / count));
            }
        }
    }
    return markers;
}







void watershed(ImageBase &gradient, vector<Vec2> &markers, vector<vector<int>> &labels) {
    int width = gradient.getWidth();
    int height = gradient.getHeight();
    int numRegions = markers.size();

    queue<Vec2> q;
    for (int i = 0; i < numRegions; i++) {
        Vec2 m = markers[i];
        labels[m[0]][m[1]] = i;
        q.push(m);
    }
    ElemStruct directions(4, {{0, 1}, {0, -1}, {1, 0}, {-1, 0}}); 


    while (!q.empty()) {
        Vec2 current = q.front();
        q.pop();
        int label = labels[current[0]][current[1]];

        for (int i=0;i<directions.taille;i++) {
            int nx = current[0] + directions.modifCo[i][0];
            int ny = current[1] + directions.modifCo[i][1];

            if (nx >= 0 && nx < height && ny >= 0 && ny < width && labels[nx][ny] == -1) {
                labels[nx][ny] = label;
                q.push(Vec2(nx, ny));
            }
        }
    }
}

void computeSuperPixelColors(ImageBase &imIn, vector<vector<int>> &labels, vector<SuperPixel> &superPixels, int minSize) {
    int width = imIn.getWidth();
    int height = imIn.getHeight();
    vector<int> pixelCount(superPixels.size(), 0);

    for (int x = 0; x < height; x++) {
        for (int y = 0; y < width; y++) {
            int label = labels[x][y];
            if (label != -1) {
                Vec3 color(imIn[x * 3][y * 3], imIn[x * 3][y * 3 + 1], imIn[x * 3][y * 3 + 2]);
                superPixels[label].meanColor += color;
                superPixels[label].pixels.push_back(Vec2(x, y));
                pixelCount[label]++;
            }
        }
    }

    for (size_t i = 0; i < superPixels.size(); i++) {
        if (pixelCount[i] > 0) {
            superPixels[i].meanColor /= pixelCount[i];
        }
    }

    for (size_t i = 0; i < superPixels.size(); i++) {
        if (pixelCount[i] < minSize && pixelCount[i] > 0) {
            map<int, double> colorDistances;

            for (const Vec2 &pixel : superPixels[i].pixels) {
                for (Vec2 dir : {Vec2(0, 1), Vec2(0, -1), Vec2(1, 0), Vec2(-1, 0)}) {
                    int nx = pixel[0] + dir[0];
                    int ny = pixel[1] + dir[1];
                    if (nx >= 0 && nx < height && ny >= 0 && ny < width) {
                        int neighborLabel = labels[nx][ny];
                        if (neighborLabel != -1 && neighborLabel != (int)i) {
                            if (colorDistances.find(neighborLabel) == colorDistances.end()) {
                                colorDistances[neighborLabel] = (superPixels[i].meanColor - superPixels[neighborLabel].meanColor).norm();
                            }
                        }
                    }
                }
            }

            int bestNeighbor = -1;
            double minDistance = DBL_MAX;
            for (const auto &entry : colorDistances) {
                if (entry.second < minDistance) {
                    minDistance = entry.second;
                    bestNeighbor = entry.first;
                }
            }

            if (bestNeighbor != -1) {
                for (const Vec2 &pixel : superPixels[i].pixels) {
                    labels[pixel[0]][pixel[1]] = bestNeighbor;
                }
                superPixels[bestNeighbor].pixels.insert(superPixels[bestNeighbor].pixels.end(),
                                                         superPixels[i].pixels.begin(),
                                                         superPixels[i].pixels.end());
                superPixels[bestNeighbor].meanColor = (superPixels[bestNeighbor].meanColor * pixelCount[bestNeighbor] +
                                                       superPixels[i].meanColor * pixelCount[i]) /
                                                      (pixelCount[bestNeighbor] + pixelCount[i]);
                pixelCount[bestNeighbor] += pixelCount[i];
                pixelCount[i] = 0;
            }
        }
    }
}

void applySuperPixelColors(ImageBase &imOut, vector<vector<int>> &labels, vector<SuperPixel> &superPixels) {
    int width = imOut.getWidth();
    int height = imOut.getHeight();

    for (int x = 0; x < height; x++) {
        for (int y = 0; y < width; y++) {
            int label = labels[x][y];
            if (label != -1) {
                Vec3 color = superPixels[label].meanColor;
                imOut[x * 3][y * 3] = color[0];
                imOut[x * 3][y * 3 + 1] = color[1];
                imOut[x * 3][y * 3 + 2] = color[2];
            }
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: ./main vader.ppm minSize\n");
        return 1;
    }
    
    std::string cNameImg = argv[1];

    int minSize = std::atoi(argv[2]);
    
    std::string cNameImgIn = "images/" + cNameImg;
    std::string cNameImgOut = "output/watershed" + cNameImg;
    
    char nameImgIn[250];
    char nameImgOur[250];
    std::strcpy(nameImgIn, cNameImgIn.c_str());
    std::strcpy(nameImgOur, cNameImgOut.c_str());
    

    ImageBase imIn;
    imIn.load(nameImgIn);
  
    int width = imIn.getWidth();
    int height = imIn.getHeight();

    ImageBase greyFlou(width, height, false);
    ImageBase grey(width, height, false);
    ImageBase gradient(width, height, false);
    ImageBase imOut(width, height, true);
  
    convertPPMtoPGM(imIn, grey);
    grey.save("./output/grey.pgm");
    ElemStruct cross(5, {{0, 0}, {0, 1}, {0, -1}, {1, 0}, {-1, 0}}); 

    filtreGaussien(grey, greyFlou, 1);
    greyFlou.save("./output/greyFlou.pgm");
    gradientSobel(greyFlou, gradient);
    gradient.save("./output/gradient.pgm");
  
    vector<Vec2> markers = findMarkers(gradient,5);
    vector<vector<int>> labels(height, vector<int>(width, -1));
    vector<SuperPixel> superPixels(markers.size());
  
    watershed(gradient, markers, labels);

    computeSuperPixelColors(imIn, labels, superPixels, minSize);

    applySuperPixelColors(imOut, labels, superPixels);

    imOut.save(nameImgOur);
  
    std::cout << "PSNR: " << ImageBase::PSNR(imIn, imOut) << std::endl;
    
    return 0;
}
