#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>
#include "json.hpp" 
#include <unordered_set>
#include <random>

#include "ImageBase.h"

using json = nlohmann::json;

void savePaletteToJson(const std::vector<std::pair<int, std::vector<double>>> &palette, const std::string &filename) {
    json j = json::array();  
    for (const auto &entry : palette) {
        j.push_back(entry.second); 
    }
    std::ofstream file(filename);
    file << j.dump(4);
    file.close();
}


std::vector<std::vector<double>> loadPaletteFromJson(const std::string &filename) {
    std::ifstream file(filename);
    json j;
    file >> j;
    file.close();

    std::vector<std::vector<double>> palette;
    for (const auto &value : j) {
        palette.push_back({value[0], value[1], value[2]});
    }
    return palette;
}


std::vector<std::vector<int>> sampleColors(const std::vector<std::vector<int>> &colors, size_t maxSamples) {
    if (colors.size() <= maxSamples) return colors;

    std::vector<std::vector<int>> sampled;
    sampled.reserve(maxSamples);
    std::unordered_set<size_t> selected;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, colors.size() - 1);

    while (sampled.size() < maxSamples) {
        size_t idx = dist(gen);
        if (selected.insert(idx).second) {
            sampled.push_back(colors[idx]);
        }
    }

    return sampled;
}

void reducePaletteWithKMeans(const std::vector<std::vector<int>> &colorsInput, int maxColors, std::vector<std::vector<double>> &palette) {
    auto colors = sampleColors(colorsInput, 5000);  
    int k = std::min(maxColors, (int)colors.size());
    std::vector<std::vector<double>> centroids(k, std::vector<double>(3, 0));
    std::vector<int> clusterAssignments(colors.size(), 0);

    for (int i = 0; i < k; ++i) {
        centroids[i] = {static_cast<double>(colors[i][0]), static_cast<double>(colors[i][1]), static_cast<double>(colors[i][2])};
    }

    bool converged = false;
    int maxIterations = 20;

    for (int iter = 0; iter < maxIterations && !converged; ++iter) {
        converged = true;

        for (size_t i = 0; i < colors.size(); ++i) {
            double minDist = std::numeric_limits<double>::max();
            int bestCluster = 0;

            for (int j = 0; j < k; ++j) {
                double dist = pow(centroids[j][0] - colors[i][0], 2) +
                              pow(centroids[j][1] - colors[i][1], 2) +
                              pow(centroids[j][2] - colors[i][2], 2);
                if (dist < minDist) {
                    minDist = dist;
                    bestCluster = j;
                }
            }

            if (clusterAssignments[i] != bestCluster) {
                converged = false;
                clusterAssignments[i] = bestCluster;
            }
        }

        std::vector<std::vector<double>> newCentroids(k, std::vector<double>(3, 0));
        std::vector<int> clusterSizes(k, 0);

        for (size_t i = 0; i < colors.size(); ++i) {
            int cluster = clusterAssignments[i];
            newCentroids[cluster][0] += colors[i][0];
            newCentroids[cluster][1] += colors[i][1];
            newCentroids[cluster][2] += colors[i][2];
            clusterSizes[cluster]++;
        }

        for (int j = 0; j < k; ++j) {
            if (clusterSizes[j] > 0) {
                newCentroids[j][0] /= clusterSizes[j];
                newCentroids[j][1] /= clusterSizes[j];
                newCentroids[j][2] /= clusterSizes[j];
            }
        }

        centroids = newCentroids;
    }

    palette = centroids;
}



void compressSuperPixelImage(ImageBase &imSuperPixel, ImageBase &imGrayscale, std::vector<std::pair<int, std::vector<double>>> &palette) {
    int height = imSuperPixel.getHeight();
    int width = imSuperPixel.getWidth();
    std::map<std::vector<int>, int> colorToIndex;
    std::vector<std::vector<int>> uniqueColors;

    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            std::vector<int> color = {
                imSuperPixel[x * 3][y * 3 + 0],
                imSuperPixel[x * 3][y * 3 + 1],
                imSuperPixel[x * 3][y * 3 + 2]
            };

            if (colorToIndex.find(color) == colorToIndex.end()) {
                colorToIndex[color] = uniqueColors.size();
                uniqueColors.push_back(color);
            }
        }
    }

    std::vector<std::vector<double>> reducedPalette;
    reducePaletteWithKMeans(uniqueColors, 256, reducedPalette);

    std::map<std::vector<int>, int> colorToPaletteIndex;

    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            std::vector<int> color = {
                imSuperPixel[x * 3][y * 3 + 0],
                imSuperPixel[x * 3][y * 3 + 1],
                imSuperPixel[x * 3][y * 3 + 2]
            };

            if (colorToPaletteIndex.find(color) == colorToPaletteIndex.end()) {
                double minDist = std::numeric_limits<double>::max();
                int bestIndex = 0;

                for (size_t i = 0; i < reducedPalette.size(); ++i) {
                    double dist = pow(reducedPalette[i][0] - color[0], 2) +
                                  pow(reducedPalette[i][1] - color[1], 2) +
                                  pow(reducedPalette[i][2] - color[2], 2);

                    if (dist < minDist) {
                        minDist = dist;
                        bestIndex = i;
                    }
                }

                colorToPaletteIndex[color] = bestIndex;
            }

            imGrayscale[x][y] = colorToPaletteIndex[color];
        }
    }

    for (size_t i = 0; i < reducedPalette.size(); ++i) {
        palette.push_back({(int)i, reducedPalette[i]});
    }

    std::sort(palette.begin(), palette.end(), [](const auto &a, const auto &b) {
        return a.first < b.first;
    });
}



void decompressSuperPixelImage(ImageBase &imGrayscale, ImageBase &imReconstructed, const std::vector<std::pair<int, std::vector<double>>> &palette) {
    int height = imGrayscale.getHeight();
    int width = imGrayscale.getWidth();

    for (int x = 0; x < height; ++x) {
        for (int y = 0; y < width; ++y) {
            int index = imGrayscale[x][y];
            const auto &color = palette[index].second;
            imReconstructed[x * 3][y * 3 + 0] = color[0];
            imReconstructed[x * 3][y * 3 + 1] = color[1];
            imReconstructed[x * 3][y * 3 + 2] = color[2];
        }
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

int main(int argc, char** argv) {
    if (argc != 5) {
        printf("Usage: ./compression mode input.ppm output.pgm palette.json\n");
        printf("Modes: compress or decompress\n");
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFile = argv[2];
    std::string outputFile = argv[3];
    std::string paletteFile = argv[4];


    std::string cNameImgIn = "output/" + inputFile;
    std::string cNameImgOut = "output/" + outputFile;
    std::string cNamepalette = "output/" + paletteFile;

    char nameImgIn[250];
    char nameImgOur[250];
    char namepalette[250];

    std::strcpy(nameImgIn, cNameImgIn.c_str());
    std::strcpy(nameImgOur, cNameImgOut.c_str());
    std::strcpy(namepalette, cNamepalette.c_str());


    ImageBase imInput;
    imInput.load(nameImgIn);

    if (mode == "compress") {
        ImageBase imGrayscale(imInput.getWidth(), imInput.getHeight(), false);
        std::vector<std::pair<int, std::vector<double>>> palette;

        compressSuperPixelImage(imInput, imGrayscale, palette);
        imGrayscale.save(nameImgOur);
        savePaletteToJson(palette, namepalette);

        std::cout << "Compression completed. Palette saved to " << paletteFile << std::endl;
    } else if (mode == "decompress") {
        ImageBase imReconstructed(imInput.getWidth(), imInput.getHeight(), true);
        std::vector<std::pair<int, std::vector<double>>> palette;

        auto rawPalette = loadPaletteFromJson(namepalette);
        for (size_t i = 0; i < rawPalette.size(); ++i) {
            palette.push_back({(int)i, rawPalette[i]});
        }

        decompressSuperPixelImage(imInput, imReconstructed, palette);
        imReconstructed.save(nameImgOur);

        std::cout << "Decompression completed. Image saved to " << outputFile << std::endl;
    } else {
        printf("Invalid mode. Use 'compress' or 'decompress'.\n");
        return 1;
    }

    return 0;
}