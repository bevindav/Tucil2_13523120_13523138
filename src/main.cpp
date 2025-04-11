#include "header/quadtree.h"
#include "header/op.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <cstdlib>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

size_t getFileSize(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    return file.tellg();
}

bool validateInputFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file.close();
    return true;
}

bool validateOutputPath(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    file.close();
    return true;
}

int main() {
    std::string inputFile;
    int errorMethod;
    double threshold;
    int minBlockSize;
    double targetCompression;
    std::string outputFile;
    std::string gifFile;
    
    // 1. [INPUT] alamat absolut gambar yang akan dikompresi
    do {
        std::cout << YELLOW << "Masukkan alamat absolut gambar yang ingin dikompresi:\n> " << RESET; 
        std::getline(std::cin, inputFile);
        if (!validateInputFile(inputFile)) {
            std::cerr << RED << "Alamat tidak valid atau file tidak ditemukan :(\n" << RESET; 
        }
    } while (!validateInputFile(inputFile));
    
    std::cout << "Pilih metode perhitungan error yang diinginkan:" << std::endl;
    std::cout << "1 - Variance" << std::endl;
    std::cout << "2 - Mean Absolute Deviation" << std::endl;
    std::cout << "3 - Max Pixel Difference" << std::endl;
    std::cout << "4 - Entropy" << std::endl;
    std::cout << "5 - Structural Similarity Index (SSIM)" << std::endl;
    do {
    std::cout << "Pilihan kamu (1-5): ";
    std::cin >> errorMethod;
    if (errorMethod < 1 || errorMethod > 5) {
        std::cerr << "Pilihan tidak valid, seharusnya antara 1-5 :(\n";
    }
    } while (errorMethod < 1 || errorMethod > 5);
    
    bool validThreshold = false;
    do {
        std::cout << "Masukkan ambang batas (threshold): ";
        std::cin >> threshold;

        validThreshold = true;

        if (threshold < 0) {
            std::cerr << "Threshold tidak bisa negatif :(" << std::endl;
            validThreshold = false;
        }
        if (errorMethod == 1 && threshold > (128*128)) {
            std::cerr << "Threshold metode Variance harus dari 0-128*128 :(" << std::endl;
            validThreshold = false;
        }
        if (errorMethod == 2 && threshold > 255) {
            std::cerr << "Threshold metode Mean Absolute Deviation harus dari 0-255 :(" << std::endl;
            validThreshold = false;
        }
        if (errorMethod == 3 && threshold > 255) {
            std::cerr << "Threshold metode Max Pixel Difference harus dari 0-255 :(" << std::endl;
            validThreshold = false;
        }
        if (errorMethod == 4 && threshold > 8) {
            std::cerr << "Threshold metode Entropy gabisa di atas 8 :(" << std::endl;
            validThreshold = false;
        }
        if (errorMethod == 5 && threshold > 1) {
            std::cerr << "Threshold metode SSIM harus 0-1 yah :(" << std::endl;
            validThreshold = false;
        }
    } while (!validThreshold);

    do {
        std::cout << "Masukkan ukuran blok minimum: ";
        std::cin >> minBlockSize;
    
        if (minBlockSize < 1) {
            std::cerr << "Ukuran blok minimal harus 1 :(" << std::endl;
        }
    } while (minBlockSize < 1);
    
    
    // [CHECK] belum implement gimmick aja duls
    do {
        std::cout << "Masukkan target kompresi (0.0-1.0, 0 untuk menonaktifkan mode ini): ";
        std::cin >> targetCompression;
    
        if (targetCompression < 0 || targetCompression > 1.0) {
            std::cerr << "Target persentase seharusnya di antara 0.0 - 1.0" << std::endl;
        }
    
    } while (targetCompression < 0 || targetCompression > 1.0);
    bool isTarget;
    if (targetCompression == 0){
        isTarget = false;
    } else {
        isTarget = true;
        minBlockSize = 1;
    }
    
    do {
        std::cin.ignore();
        std::cout << "Masukkan alamat absolut gambar hasil kompresi: ";
        std::getline(std::cin, outputFile);
    
        if (!validateOutputPath(outputFile)) {
            std::cerr << "Gagal write file :(" << std::endl;
        }
    
    } while (!validateOutputPath(outputFile));
    
    std::cout << "Masukkan alamat absolut GIF: ";
    std::getline(std::cin, gifFile);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<Color>> image;
    int width, height;
    if (!readImage(inputFile, image, width, height)) {
        std::cerr << "Gagal read gambar :(" << std::endl;
        return 1;
    }
    
    size_t originalSize = getFileSize(inputFile);

    if (isTarget) {
        threshold = estimateThresholdForTargetCompression(image, errorMethod, minBlockSize, targetCompression, originalSize);
    }

    QuadTree quadtree;
    quadtree.buildfrImage(image, errorMethod, threshold, minBlockSize);
    
    std::vector<std::vector<Color>> reconstructedImage = quadtree.reconstructImage(width, height);
    
    if (!writeImage(outputFile, reconstructedImage)) {
        std::cerr << "Gagal write output :(" << std::endl;
        return 1;
    }

    if (!gifFile.empty()) {
        std::cout << "Memroses GIF..." << std::endl;
        createQuadtreeGIF(gifFile, image, quadtree, errorMethod, threshold, minBlockSize);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    
    // [CHECK] 
    size_t compressedSize = getFileSize(outputFile);
    double compressionPercentage = (1.0 - static_cast<double>(compressedSize) / originalSize) * 100.0;
    
    //output
    const int lebarTabel = 52;

    auto printLine = [](char ch = '-') {
        std::cout << CYAN << "+" << std::string(lebarTabel - 2, ch) << "+" << RESET << std::endl;
    };

    auto printCentered = [&](const std::string& msg) {
        int padding = (lebarTabel - 2 - msg.length()) / 2;
        std::cout << CYAN << "|" << std::string(padding, ' ') 
                << BOLD << msg << RESET 
                << std::string(lebarTabel - 2 - padding - msg.length(), ' ') 
                << CYAN << "|" << RESET << std::endl;
    };

    auto printRow = [&](const std::string& label, const std::string& value, const std::string& color = RESET) {
        int labelWidth = 24;
        int valueWidth = lebarTabel - 2 - labelWidth;
        std::cout << CYAN << "| " << RESET
                << std::left << std::setw(labelWidth) << label << ": "
                << color << std::right << std::setw(valueWidth - 2) << value << RESET
                << " " << CYAN << "|" << RESET << std::endl;
    };

    printLine();
    printCentered("HASIL KOMPRESI QUADTREE");
    printLine();

    printRow("Waktu eksekusi", std::to_string(duration) + " ms", GREEN);
    printRow("Ukuran gambar awal", std::to_string(originalSize) + " bytes", YELLOW);
    printRow("Ukuran gambar akhir", std::to_string(compressedSize) + " bytes", YELLOW);
    std::ostringstream compressionStream;
    compressionStream << std::fixed << std::setprecision(2) << compressionPercentage << " %";
    printRow("Persentase kompresi", compressionStream.str(), MAGENTA);
    printRow("Kedalaman pohon", std::to_string(quadtree.getMaxDepth()), BLUE);
    printRow("Banyak simpul", std::to_string(quadtree.getTotalNodes()), BLUE);
    printLine();

    printRow("Gambar tersimpan di", outputFile);
    printLine();

    if (!gifFile.empty()) {
        printRow("GIF tersimpan di", gifFile);
        printLine();
    }
    return 0;
}