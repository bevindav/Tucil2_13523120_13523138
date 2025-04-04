#ifndef OP_H
#define OP_H
#include "quadtree.h"
#include <vector>
#include <string>

Color hitungAverageColor(const std::vector<Color>& pixels);

// Variance
double hitungVariance(const std::vector<Color>& pixels, const Color& avgColor);

// Mean Absolute Deviation
double hitungMAD(const std::vector<Color>& pixels, const Color& avgColor);

// Max Pixel Difference
double hitungMaxDifference(const std::vector<Color>& pixels);

// Entropy
double hitungEntropy(const std::vector<Color>& pixels);

// konversi format
std::string getFileExtension(const std::string& filename);
bool convertToPPM(const std::string& inputFile, const std::string& outputPPM);
bool convertFromPPM(const std::string& inputPPM, const std::string& outputFile);

//image
bool readImage(const std::string& filename, std::vector<std::vector<Color>>& image, int& width, int& height);

bool writeImage(const std::string& filename, const std::vector<std::vector<Color>>& image);

double calculateCompressionPercentage(int originalSize, int compressedSize);

#endif