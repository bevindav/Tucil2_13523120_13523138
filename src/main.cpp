#include "header/quadtree.h"
#include "header/op.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <fstream>

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
        std::cerr << "Gagal open file :(" << path << std::endl;
        return false;
    }
    file.close();
    return true;
}

bool validateOutputPath(const std::string& path) {
    if (path.empty()) {
        std::cerr << "Alamat absolut tidak bisa kosong :(" << std::endl;
        return false;
    }
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Gagal write file :(" << path << std::endl;
        return false;
    }
    file.close();
    return true;
}

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    }

void displayProgressBar(int progress, int total) {
    const int barWidth = 50;
    float ratio = static_cast<float>(progress) / total;
    int pos = barWidth * ratio;
    
    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(ratio * 100.0) << "% ";
    
    if (progress == total) {
        std::cout << std::endl;
    }
    std::cout.flush();
}


int main() {
    std::string inputFile;
    int errorMethod;
    double threshold;
    int minBlockSize;
    double targetCompression; // [CHECK] gimmick aja bang
    std::string outputFile;
    std::string gifFile; // [CHECK] gimmick aja bang
    
    // 1. [INPUT] alamat absolut gambar yang akan dikompresi
    std::cout << "Masukkan alamat absolut gambar yang akan dikompresi: ";
    std::cin >> inputFile;
    if (!validateInputFile(inputFile)) {
        return 1;
    }
    
    std::cout << "Pilih metode perhitungan error yang diinginkan:" << std::endl;
    std::cout << "1 - Variance" << std::endl;
    std::cout << "2 - Mean Absolute Deviation" << std::endl;
    std::cout << "3 - Max Pixel Difference" << std::endl;
    std::cout << "4 - Entropy" << std::endl;
    std::cout << "Pilihan kamu: ";
    std::cin >> errorMethod;
    if (errorMethod < 1 || errorMethod > 4) {
        std::cerr << "Pilihanmu seharusnya 1-4 :(" << std::endl;
        return 1;
    }
    
    // [CHECK] ini batasnya ga negatif aja apa gimana ya..
    std::cout << "Masukkan ambang batas (threshold): ";
    std::cin >> threshold;
    if (threshold < 0) {
        std::cerr << "Threshold tidak bisa negatif :(" << std::endl;
        return 1;
    }
    std::cout << "Masukkan ukuran blok minimum: ";
    std::cin >> minBlockSize;
    if (minBlockSize < 1) {
        std::cerr << "Ukuran blok minimal harus 1 :(" << std::endl;
        return 1;
    }
    
    // [CHECK] belum implement gimmick aja duls
    std::cout << "Masukkan target kompresi (0.0-1.0, 0 untuk menonaktifkan mode ini): ";
    std::cin >> targetCompression;
    if (targetCompression < 0 || targetCompression > 1.0) {
        std::cerr << "Target persentase seharusnya di antara 0.0 - 1.0" << std::endl;
        return 1;
    }
    
    std::cout << "Masukkan alamat absolut gambar hasil kompresi: ";
    std::cin >> outputFile;
    if (!validateOutputPath(outputFile)) {
        return 1;
    }
    
    // [CHECK] belum implement gimmick aja duls
    std::cout << "Masukkan alamat absolut GIF (enter aja dlu untuk skip): ";
    std::cin.ignore();
    std::getline(std::cin, gifFile);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<Color>> image;
    int width, height;
    if (!readImage(inputFile, image, width, height)) {
        std::cerr << "Gagal read gambar :(" << std::endl;
        return 1;
    }
    
    size_t originalSize = getFileSize(inputFile);
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
    std::cout << "\n+--------------------------------------------------+" << std::endl;
    std::cout << "|              HASIL KOMPRESI QUADTREE              |" << std::endl;
    std::cout << "+--------------------------------------------------+" << std::endl;
    std::cout << "| Waktu eksekusi       : " << std::setw(10) << duration << " ms";
    std::cout << std::string(15, ' ') << "|" << std::endl;
    
    std::cout << "| Ukuran gambar awal   : " << std::setw(10) << originalSize << " bytes";
    std::cout << std::string(12, ' ') << "|" << std::endl;
    
    std::cout << "| Ukuran gambar akhir  : " << std::setw(10) << compressedSize << " bytes";
    std::cout << std::string(10, ' ') << "|" << std::endl;
    
    std::cout << "| Persentase kompresi  : " << std::fixed << std::setprecision(2) 
              << std::setw(10) << compressionPercentage << " %"; //[CHECK] gimik ga sih apa udah bener
    std::cout << std::string(15, ' ') << "|" << std::endl;
    
    std::cout << "| Kedalaman pohon      : " << std::setw(10) << quadtree.getMaxDepth();
    std::cout << std::string(19, ' ') << "|" << std::endl;
    
    std::cout << "| Banyak simpul        : " << std::setw(10) << quadtree.getTotalNodes();
    std::cout << std::string(19, ' ') << "|" << std::endl;
    
    std::cout << "+--------------------------------------------------+" << std::endl;
    std::cout << "| Gambar berhasil tersimpan di: " << outputFile;
    std::cout << std::string(std::max(0, 24 - (int)outputFile.length()), ' ') << "|" << std::endl;
    std::cout << "+--------------------------------------------------+" << std::endl;
    if (!gifFile.empty()) {
        std::cout << "+--------------------------------------------------+" << std::endl;
        std::cout << "| GIF berhasil tersimpan di: " << gifFile;
        std::cout << std::string(std::max(0, 24 - (int)outputFile.length()), ' ') << "|" << std::endl;
        std::cout << "+--------------------------------------------------+" << std::endl;
    }
    return 0;
}