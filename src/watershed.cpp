#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <queue>
#include <map>
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


void gadients(ImageBase &imIn,ImageBase &normeGradient){
    for(int x=0;x<imIn.getHeight();x++){
        for(int y=0;y<imIn.getWidth();y++){	

            float hori=(y+1)<imIn.getWidth() ? imIn[x][y+1]-imIn[x][y] : 0 ;
            float ver=(x+1)<imIn.getHeight() ? imIn[x+1][y]-imIn[x][y] : 0;
            float norme=sqrt(pow(hori,2)+pow(ver,2));

            normeGradient[x][y]=clamp((int)(norme),0,255);;

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



void convertPPMtoPGM(ImageBase &imIn, ImageBase &Y) {
    for (int x = 0; x < imIn.getHeight(); ++x) {
        for (int y = 0; y < imIn.getWidth(); ++y) {
            Y[x][y] = 0.3 * imIn[x * 3][y * 3] + 
                      0.6 * imIn[x * 3][y * 3 + 1] + 
                      0.1 * imIn[x * 3][y * 3 + 2];       
        }
    }
}

vector<Vec2> findMarkers(ImageBase &gradient) {
    vector<Vec2> markers;
    int width = gradient.getWidth();
    int height = gradient.getHeight();

    for (int x = 1; x < height - 1; x++) {
        for (int y = 1; y < width - 1; y++) {
            int g = gradient[x][y];
            if (g < gradient[x - 1][y] && g < gradient[x + 1][y] &&
                g < gradient[x][y - 1] && g < gradient[x][y + 1]) {
                markers.push_back(Vec2(x, y));
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

void computeSuperPixelColors(ImageBase &imIn, vector<vector<int>> &labels, vector<SuperPixel> &superPixels) {
    int width = imIn.getWidth();
    int height = imIn.getHeight();
    vector<int> pixelCount(superPixels.size(), 0);

    for (int x = 0; x < height; x++) {
        for (int y = 0; y < width; y++) {
            int label = labels[x][y];
            if (label != -1) {
                Vec3 color(imIn[x * 3][y * 3], imIn[x * 3][y * 3 + 1], imIn[x * 3][y * 3 + 2]);
                superPixels[label].meanColor += color;
                pixelCount[label]++;
            }
        }
    }

    for (size_t i = 0; i < superPixels.size(); i++) {
        if (pixelCount[i] > 0) {
            superPixels[i].meanColor /= pixelCount[i];
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
    if (argc != 2) {
        printf("Usage: ./main vader.ppm\n");
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
  
    int width = imIn.getWidth();
    int height = imIn.getHeight();

    ImageBase greyFlou(width, height, false);
    ImageBase grey(width, height, false);
    ImageBase gradient(width, height, false);
    ImageBase imOut(width, height, true);
  
    convertPPMtoPGM(imIn, grey);

    ElemStruct cross(9, {{0, 0}, {0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}}); 
    filtreMoy(grey, greyFlou, cross);
    gadients(greyFlou, gradient);
    gradient.save("./output/gradient.pgm");
  
    vector<Vec2> markers = findMarkers(gradient);
    vector<vector<int>> labels(height, vector<int>(width, -1));
    vector<SuperPixel> superPixels(markers.size());
  
    watershed(gradient, markers, labels);
    computeSuperPixelColors(imIn, labels, superPixels);
    applySuperPixelColors(imOut, labels, superPixels);
  
    imOut.save(nameImgOur);
  
    return 0;
}
