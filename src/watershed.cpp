#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <numeric>  
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



void histo(ImageBase &imIn,std::vector<int>& histo){
    int tab[256]={};
    for(int x = 0; x < imIn.getHeight(); ++x)
    for(int y = 0; y < imIn.getWidth(); ++y)
    {
        tab[imIn[x][y]]++;
    }

    for(int i=0;i<256;i++){
        histo[i]=tab[i];
    }	

}

int otsu(const std::vector<int>& histo, int totalPixels){

    std::vector<double> proba(256,0);
    for(int i=0;i<256;i++){
        proba[i]=(double)histo[i]/(double)totalPixels;
    }	

    double moyenneImage=0;
    for(double j=0.;j<256.;j++){
        moyenneImage+=j*proba[j];
        
    }

    double maxVarience=0;
    int meilleurSeuil=0;

    for(int t=1;t<255;t++){

        double probC1= 0;
        double moyC1=0;

        for(int i=0;i<t;i++){
            probC1+=proba[i];
            moyC1+=(double)i*proba[i];

        }
        double probC2=1.-probC1;

        double var= pow((moyenneImage*probC1-moyC1),2)/(probC1*probC2);

        if(var>maxVarience){
            maxVarience=var;
            meilleurSeuil=t;
        }
    }

    return meilleurSeuil;
}




void seuilGris( ImageBase &imIn, ImageBase &imOut, int S){
    for(int x = 0; x < imIn.getHeight(); ++x)
    for(int y = 0; y < imIn.getWidth(); ++y)
    {
        if (imIn[x][y] < S) 
            imOut[x][y] = 0;
        else imOut[x][y] = 255;
    }
}



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


    

void gadients(ImageBase &imIn,ImageBase &normeGradient){
    for(int x=0;x<imIn.getHeight();x++){
        for(int y=0;y<imIn.getWidth();y++){	

            float hori=(y+1)<imIn.getWidth() ? imIn[x][y+1]-imIn[x][y] : 0 ;
            float ver=(x+1)<imIn.getHeight() ? imIn[x+1][y]-imIn[x][y] : 0;
            float norme=sqrt(pow(hori,2)+pow(ver,2));

            normeGradient[x][y]=clamp((int)(norme+128),0,255);;

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

void convertPPMtoPGM(ImageBase &imIn,ImageBase &Y){
    for (int x = 0; x < imIn.getHeight(); ++x) {
        for (int y = 0; y < imIn.getWidth(); ++y) {
            Y[x][y]=0.3*(float)imIn[x*3][y*3]+0.6*(float)imIn[x*3][y*3+1]+0.1*(float)imIn[x*3][y*3+2];       
        }
    }
}



void watershed(ImageBase &imIn,ImageBase &imgradient,ImageBase &imOut){


    
}




int main(int argc, char** argv) {


    std::string cNameImg = argv[1];

    std::string cNameImgIn = "images/" + cNameImg;
    std::string cNameImgOut = "output/" + cNameImg;

    ImageBase imIn,imOut;

    char nameImgIn[250];
    char nameImgOur[250];

    std::strcpy(nameImgIn, cNameImgIn.c_str());
    std::strcpy(nameImgOur, cNameImgOut.c_str());

    imIn.load(nameImgIn);

    ImageBase grey(imIn.getWidth(),imIn.getHeight(),false);
    ImageBase grad(imIn.getWidth(),imIn.getHeight(),false);
    ImageBase greyFlou(imIn.getWidth(),imIn.getHeight(),false);
    ImageBase gradSeuil(imIn.getWidth(),imIn.getHeight(),false);

    convertPPMtoPGM(imIn,grey);


    ElemStruct cross(5,{{0,0},{0,1},{0,-1},{1,0},{-1,0}}); 

    filtreMoy(grey,greyFlou,cross);

    gadients(greyFlou,grad);

    vector<int> histoVec(256,0);

    histo(grad,histoVec);


    seuilGris(grad,gradSeuil,163);

    gradSeuil.save(nameImgOur);


    watershed(imIn,grad,imOut);

    return 0;
}
