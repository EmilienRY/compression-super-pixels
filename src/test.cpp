#include "./src/ImageBase.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <numeric>
#include <cmath> 
#include <limits>
#include <random>
using namespace std;


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




	void kmean(int nbPass, ImageBase &imIn, ImageBase &imOut1, ImageBase &imOut2,ImageBase &imOut3, std::vector<std::vector<double>> &colors,std::vector<std::vector<double>> &paletteFinale) {
		int k = colors.size(); 
		bool continu=true;
		std::vector<std::vector<double>> centroids = colors; 
		std::vector<int> clusters(imIn.getHeight() * imIn.getWidth(), 0); 
		int pass=0;
		while (continu && pass<nbPass) {
			std::vector<std::vector<double>> newCentroids(k, std::vector<double>(3, 0));
			std::vector<int> clusterCounts(k, 0);

			for (int x = 0; x < imIn.getHeight(); ++x) {
				for (int y = 0; y < imIn.getWidth(); ++y) {
					int r = imIn[x * 3][y * 3 + 0];
					int g = imIn[x * 3][y * 3 + 1];
					int b = imIn[x * 3][y * 3 + 2];

					double minDist = std::numeric_limits<double>::max();
					int bestCluster = 0;

					for (int i = 0; i < k; ++i) {
						double dist = pow(centroids[i][0] - r, 2) +
									pow(centroids[i][1] - g, 2) +
									pow(centroids[i][2] - b, 2);

						if (dist < minDist) {
							minDist = dist;
							bestCluster = i;
						}
					}

					clusters[x * imIn.getWidth() + y] = bestCluster;

					newCentroids[bestCluster][0] += r;
					newCentroids[bestCluster][1] += g;
					newCentroids[bestCluster][2] += b;
					clusterCounts[bestCluster]++;
				}
			}
			continu=false;
			for (int i = 0; i < k; ++i) {
				if (clusterCounts[i] > 0) {
					newCentroids[i][0] = newCentroids[i][0] / clusterCounts[i];
					newCentroids[i][1] = newCentroids[i][1] / clusterCounts[i];
					newCentroids[i][2] = newCentroids[i][2] / clusterCounts[i];


					if((std::abs(newCentroids[i][0]-centroids[i][0])>=1 && std::abs(newCentroids[i][1]-centroids[i][1])>=1 && std::abs(newCentroids[i][2]-centroids[i][2])>=1 )){
						continu=true;
					}

					centroids[i][0] = newCentroids[i][0] ;
					centroids[i][1] = newCentroids[i][1] ;
					centroids[i][2] = newCentroids[i][2] ;
				}
			}
			pass+=1;
		}
		paletteFinale=centroids;
		for (int x = 0; x < imIn.getHeight(); ++x) {
			for (int y = 0; y < imIn.getWidth(); ++y) {
				int cluster = clusters[x * imIn.getWidth() + y];
				imOut1[x * 3][y * 3 + 0] = colors[cluster][0];
				imOut1[x * 3][y * 3 + 1] = colors[cluster][1];
				imOut1[x * 3][y * 3 + 2] = colors[cluster][2];

				imOut2[x * 3][y * 3 + 0] = centroids[cluster][0];
				imOut2[x * 3][y * 3 + 1] = centroids[cluster][1];
				imOut2[x * 3][y * 3 + 2] = centroids[cluster][2];

				imOut3[x][y]= cluster;

			}
		}

		printf("nb pass fin %d\n",pass);
	}



	void generateUniformColors(std::vector<std::vector<double>> &colors, int numColors) {
		int step = 256 / std::cbrt(numColors); 

		colors.resize(numColors, std::vector<double>(3, 0)); 
		int index = 0;

		for (int r = 0; r < 256; r += step) {
			for (int g = 0; g < 256; g += step) {
				for (int b = 0; b < 256; b += step) {
					if (index < numColors) {
						colors[index][0] = r;
						colors[index][1] = g;
						colors[index][2] = b;
						++index;
					} else {
						return;
					}
				}
			}
		}
	}



	void reconstruct(std::vector<std::vector<double>> &palette,ImageBase &imGris, ImageBase &imCouleur){

		for (int x = 0; x < imGris.getHeight(); ++x) {
			for (int y = 0; y < imGris.getWidth(); ++y) {

				vector<double> color = palette[imGris[x][y]];

				imCouleur[3*x][3*y] = color[0];
				imCouleur[3*x][3*y+1] = color[1];
				imCouleur[3*x][3*y+2] =color[2];


			}
		}
	}



	int main(int argc, char **argv)
	{
		char cNomImgLue[250], cNomImgEcrite[250],cNomImgEcrite2[250],cNomImgEcrite3[250],cNomImgEcrite4[250];


		sscanf (argv[1],"%s",cNomImgLue) ;
		sscanf (argv[2],"%s",cNomImgEcrite);
		sscanf (argv[3],"%s",cNomImgEcrite2);
		sscanf (argv[4],"%s",cNomImgEcrite3);
		sscanf (argv[5],"%s",cNomImgEcrite4);

		ImageBase imIn;
		imIn.load(cNomImgLue);

		ImageBase imOut(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
		ImageBase imOut2(imIn.getWidth(), imIn.getHeight(), imIn.getColor());
		ImageBase imOut3(imIn.getWidth(), imIn.getHeight(), false);
		ImageBase imOut4(imIn.getWidth(), imIn.getHeight(), true);


		int numColors = 128; 
		std::vector<std::vector<double>> colors;
		std::vector<std::vector<double>> palette;

		generateUniformColors(colors, numColors);
		kmean(15,imIn,imOut,imOut2,imOut3,colors,palette);

		reconstruct(palette,imOut3,imOut4);

		imOut.save(cNomImgEcrite);
		imOut2.save(cNomImgEcrite2);
		imOut3.save(cNomImgEcrite3);
		imOut4.save(cNomImgEcrite4);

		double psnr=PSNR(imIn,imOut2);
		printf("le PSNR est de %f db\n ",psnr);
		

		return 0;
	}
