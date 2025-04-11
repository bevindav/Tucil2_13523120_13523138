#define STB_IMAGE_IMPLEMENTATION
#include "header/stb_image.h" 
#include "header/quadtree.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "header/stb_image_write.h" 
#define GIF_STATIC
#define GIF_IMPL
#include "header/gif.h"
#include "header/op.h"
#include <cmath>
#include <algorithm>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <string>

Color hitungAverageColor(const std::vector<Color>& pixels) {
    if (pixels.empty()) return Color(0, 0, 0);
    
    unsigned long long sumR = 0, sumG = 0, sumB = 0;

    for (const auto& pixel : pixels) {
        sumR += pixel.r;
        sumG += pixel.g;
        sumB += pixel.b;
    }
    
    int count = pixels.size();
    return Color(sumR / count, sumG / count, sumB / count);
}

double hitungVariance(const std::vector<Color>& pixels, const Color& avgColor) {
    if (pixels.empty()) return 0.0;
    
    double sumSqrDiffR = 0.0, sumSqrDiffG = 0.0, sumSqrDiffB = 0.0;
    
    for (const auto& pixel : pixels) {
        double diffR = pixel.r - avgColor.r;
        double diffG = pixel.g - avgColor.g;
        double diffB = pixel.b - avgColor.b;
        
        sumSqrDiffR += diffR * diffR;
        sumSqrDiffG += diffG * diffG;
        sumSqrDiffB += diffB * diffB;
    }
    
    int count = pixels.size();
    double varianceR = sumSqrDiffR / count;
    double varianceG = sumSqrDiffG / count;
    double varianceB = sumSqrDiffB / count;
    
    return (varianceR + varianceG + varianceB) / 3.0;
}

double hitungMAD(const std::vector<Color>& pixels, const Color& avgColor) {
    if (pixels.empty()) return 0.0;
    
    double sumAbsDiffR = 0.0, sumAbsDiffG = 0.0, sumAbsDiffB = 0.0;
    
    for (const auto& pixel : pixels) {
        sumAbsDiffR += std::abs(pixel.r - avgColor.r);
        sumAbsDiffG += std::abs(pixel.g - avgColor.g);
        sumAbsDiffB += std::abs(pixel.b - avgColor.b);
    }
    
    int count = pixels.size();
    double madR = sumAbsDiffR / count;
    double madG = sumAbsDiffG / count;
    double madB = sumAbsDiffB / count;
    
    return (madR + madG + madB) / 3.0;
}

double hitungMaxDifference(const std::vector<Color>& pixels) {
    if (pixels.empty()) return 0.0;
    
    unsigned char minR = 255, minG = 255, minB = 255;
    unsigned char maxR = 0, maxG = 0, maxB = 0;
    
    for (const auto& pixel : pixels) {
        minR = std::min(minR, pixel.r);
        minG = std::min(minG, pixel.g);
        minB = std::min(minB, pixel.b);
        
        maxR = std::max(maxR, pixel.r);
        maxG = std::max(maxG, pixel.g);
        maxB = std::max(maxB, pixel.b);
    }
    
    double diffR = maxR - minR;
    double diffG = maxG - minG;
    double diffB = maxB - minB;
    
    return (diffR + diffG + diffB) / 3.0;
}

double hitungEntropy(const std::vector<Color>& pixels) {
    if (pixels.empty()) return 0.0;
    
    std::map<unsigned char, int> histR, histG, histB;
    
    for (const auto& pixel : pixels) {
        histR[pixel.r]++;
        histG[pixel.g]++;
        histB[pixel.b]++;
    }
    
    double entropyR = 0.0, entropyG = 0.0, entropyB = 0.0;
    int count = pixels.size();
    
    for (const auto& pair : histR) {
        double probability = static_cast<double>(pair.second) / count;
        entropyR -= probability * std::log2(probability);
    }
    
    for (const auto& pair : histG) {
        double probability = static_cast<double>(pair.second) / count;
        entropyG -= probability * std::log2(probability);
    }
    
    for (const auto& pair : histB) {
        double probability = static_cast<double>(pair.second) / count;
        entropyB -= probability * std::log2(probability);
    }
    
    return (entropyR + entropyG + entropyB) / 3.0;
}

double hitungSSIM(const std::vector<Color>& pixels, const Color& avgColor) {
    if (pixels.empty()) return 0.0;

    double L = 255;
    double C1 = (0.03 * L) * (0.03 * L);

    double sumSqrDiffR = 0.0, sumSqrDiffG = 0.0, sumSqrDiffB = 0.0;

    for (const auto& pixel : pixels) {
        double diffR = pixel.r - avgColor.r;
        double diffG = pixel.g - avgColor.g;
        double diffB = pixel.b - avgColor.b;
        
        sumSqrDiffR += diffR * diffR;
        sumSqrDiffG += diffG * diffG;
        sumSqrDiffB += diffB * diffB;
    }
    
    int count = pixels.size();
    double varianceR = sumSqrDiffR / count;
    double varianceG = sumSqrDiffG / count;
    double varianceB = sumSqrDiffB / count;

    // SSIM per channel
    double ssimR = C1 / (varianceR + C1);
    double ssimG = C1 / (varianceG + C1);
    double ssimB = C1 / (varianceB + C1);

    return (ssimR + ssimG + ssimB) / 3.0;
}

size_t getFileSize(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    return file.tellg();
}

std::string getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == filename.length() - 1) {
        return "";
    }
    
    std::string ext = filename.substr(dotPos + 1);
    for (char& c : ext) {
        c = std::tolower(c);
    }
    
    return ext;
}

bool readImage(const std::string& filename, std::vector<std::vector<Color>>& image, int& width, int& height) {
    int channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 3);
    
    if (!data) {
        std::cerr << "Gagal memuat gambar:" << filename << std::endl;
        std::cerr << "Debug STB: " << stbi_failure_reason() << std::endl;
        return false;
    }
    image.resize(height, std::vector<Color>(width));
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            image[y][x] = Color(data[index], data[index + 1], data[index + 2]);
        }
    }
    stbi_image_free(data);
    return true;
}

bool writeImage(const std::string& filename, const std::vector<std::vector<Color>>& image) {
    if (image.empty() || image[0].empty()) {
        std::cerr << "Gambarnya kosong :(" << std::endl;
        return false;
    }
    std::cout << "Memroses gambar..." << std::endl;
    int height = image.size();
    int width = image[0].size();
    std::string extension = getFileExtension(filename);
    
    unsigned char* data = new unsigned char[width * height * 3];
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            data[index] = image[y][x].r;
            data[index + 1] = image[y][x].g;
            data[index + 2] = image[y][x].b;
        }
    }
    
    bool success = false;
    
    if (extension == "png") {
        success = stbi_write_png(filename.c_str(), width, height, 3, data, width * 3);
    } 
    else if (extension == "jpg" || extension == "jpeg") {
        success = stbi_write_jpg(filename.c_str(), width, height, 3, data, 90); // 90 kualitas (0-100)
    }
    else if (extension == "bmp") {
        success = stbi_write_bmp(filename.c_str(), width, height, 3, data);
    }
    else if (extension == "tga") {
        success = stbi_write_tga(filename.c_str(), width, height, 3, data);
    }
    else {
        std::cerr << "Format tidak didukung :(" << extension << std::endl;
        std::cerr << "Format sudah salah satu dari png, jpg, jpeg, bmp, tga belum? :)" << std::endl;
        delete[] data;
        return false;
    }
    
    delete[] data;
    
    if (!success) {
        std::cerr << "Gagal write gambar: " << filename << std::endl;
        return false;
    }
    return true;
}

void createQuadtreeGIF(
    const std::string& outputGifPath,
    const std::vector<std::vector<Color>>& originalImage,
    QuadTree& quadtree,
    int errorMethod,
    double threshold,
    int minBlockSize
) {
    int maxDepth = quadtree.getMaxDepth();
    int width = originalImage[0].size();
    int height = originalImage.size();
        
    GifWriter gifWriter = {};
    GifBegin(&gifWriter, outputGifPath.c_str(), width, height, 100); // 100ms per frame
    
    for (int depth = 0; depth <= maxDepth; depth++) {
        std::vector<std::vector<Color>> frameImage = quadtree.reconstructImageForGIF(depth);
        
        std::vector<uint8_t> rgbaPixels(width * height * 4);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;
                
                if (y < frameImage.size() && x < frameImage[y].size()) {
                    Color c = frameImage[y][x];
                    rgbaPixels[idx] = c.r;     
                    rgbaPixels[idx + 1] = c.g; 
                    rgbaPixels[idx + 2] = c.b; 
                    rgbaPixels[idx + 3] = 255; 
                } else {
                    rgbaPixels[idx] = 0;
                    rgbaPixels[idx + 1] = 0;
                    rgbaPixels[idx + 2] = 0;
                    rgbaPixels[idx + 3] = 255;
                }
            }
        }
        GifWriteFrame(&gifWriter, rgbaPixels.data(), width, height, 100);
    }
}

double estimateThresholdForTargetCompression(
    const std::vector<std::vector<Color>>& image, 
    int errorMethod, 
    int minBlockSize, 
    double targetCompression,
    int originalSize
) {
    double low = 0.0;
    double high = (errorMethod == 1) ? 128 * 128 : 1.0; // max threshold tergantung metode
    if (errorMethod == 2 || errorMethod == 3) high = 255;
    if (errorMethod == 4) high = 8.0;

    double tolerance = 0.01; // 1% toleransi
    double bestThreshold = (errorMethod == 5) ? high / 2 : low;

    for (int i = 0; i < 20; i++) {
        // std::cout << "Mencari threshold... " << (i+1) << std::endl;
        double mid = (low + high) / 2.0;

        QuadTree qt;
        qt.buildfrImage(image, errorMethod, mid, minBlockSize);
        std::vector<std::vector<Color>> reconstructed = qt.reconstructImage(image[0].size(), image.size());

        std::string tempOut = "temp.jpg";
        writeImage(tempOut, reconstructed);
        int compressedSize = getFileSize(tempOut);
        std::remove(tempOut.c_str());

        double compressionRatio = 1.0 - static_cast<double>(compressedSize) / originalSize;

        // std::cout << "Threshold: " << mid << std::endl;

        if (std::abs(compressionRatio - targetCompression) <= tolerance) {
            bestThreshold = mid;
            break;
        }
        
        if (errorMethod==5) {
            if (compressionRatio < targetCompression) {
                low = mid;
            } else {
                high = mid;
            }
        } else {
            if (compressionRatio < targetCompression) {
                high = mid;
            } else {
                low = mid;
            }
        }
        bestThreshold = mid;
    }
    // std::cout << "Final threshold" << bestThreshold << std::endl;
    return bestThreshold;
}