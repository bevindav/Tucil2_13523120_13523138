#include "header/quadtree.h"
#include "header/op.h"
#include <algorithm>
#include <cmath>

QuadTreeNode::QuadTreeNode(int x, int y, int panjang, int lebar)
    : x(x), y(y), panjang(panjang), lebar(lebar), isLeaf(true),
      topLeft(nullptr), topRight(nullptr), bottomLeft(nullptr), bottomRight(nullptr) {}

QuadTreeNode::~QuadTreeNode() {
    if (topLeft) delete topLeft;
    if (topRight) delete topRight;
    if (bottomLeft) delete bottomLeft;
    if (bottomRight) delete bottomRight;
}

void QuadTreeNode::split() {
    int halfpanjang = panjang / 2;
    int halflebar = lebar / 2;
    
    topLeft = new QuadTreeNode(x, y, halfpanjang, halflebar);
    topRight = new QuadTreeNode(x + halfpanjang, y, halfpanjang, halflebar);
    bottomLeft = new QuadTreeNode(x, y + halflebar, halfpanjang, halflebar);
    bottomRight = new QuadTreeNode(x + halfpanjang, y + halflebar, halfpanjang, halflebar);
    
    isLeaf = false;
}

bool QuadTreeNode::hasChildren() const {
    return !isLeaf && topLeft != nullptr;
}

QuadTree::QuadTree() : root(nullptr), totalN(0), maxDepth(0) {}

QuadTree::~QuadTree() {
    if (root) delete root;
}

void QuadTree::buildfrImage(const std::vector<std::vector<Color>>& image, int errorMethod, double threshold, int minBlockSize) {
    if (image.empty() || image[0].empty()) return;
    
    int panjang = image[0].size();
    int lebar = image.size();
    
    int size = std::max(panjang, lebar);
    size = std::pow(2, std::ceil(std::log2(size)));
    
    root = new QuadTreeNode(0, 0, size, size);
    totalN = 1;
    maxDepth = 0;
    
    buildNode(root, image, errorMethod, threshold, minBlockSize, 0);
}

void QuadTree::buildNode(QuadTreeNode* node, const std::vector<std::vector<Color>>& image, 
                        int errorMethod, double threshold, int minBlockSize, int depth) {
    if (!node) return;
    
    maxDepth = std::max(maxDepth, depth);
    
    int nodeX = node->getX();
    int nodeY = node->getY();
    int nodepanjang = node->getpanjang();
    int nodelebar = node->getlebar();
    
    std::vector<Color> blockPixels;
    for (int y = nodeY; y < nodeY + nodelebar && y < image.size(); y++) {
        for (int x = nodeX; x < nodeX + nodepanjang && x < image[0].size(); x++) {
            if (y < image.size() && x < image[y].size()) {
                blockPixels.push_back(image[y][x]);
            } else {
                blockPixels.push_back(Color(0, 0, 0));
            }
        }
    }
    
    //warna rata-rata 
    Color avgColor = hitungAverageColor(blockPixels);
    node->setAvgColor(avgColor);
    
    double error = 0;
    switch (errorMethod) {
        case 1: // Variance
            error = hitungVariance(blockPixels, avgColor);
            break;
        case 2: // Mean Absolute Deviance
            error = hitungMAD(blockPixels, avgColor);
            break;
        case 3: // Max Pixel Difference
            error = hitungMaxDifference(blockPixels);
            break;
        case 4: // Entropy
            error = hitungEntropy(blockPixels);
            break;
        default:
            error = hitungVariance(blockPixels, avgColor);
    }
    
    bool splitKah = error > threshold && 
                      nodepanjang >= 2 * minBlockSize && 
                      nodelebar >= 2 * minBlockSize;
    
    if (splitKah) {
        node->split();
        totalN += 4;
        
        buildNode(node->getTopLeft(), image, errorMethod, threshold, minBlockSize, depth + 1);
        buildNode(node->getTopRight(), image, errorMethod, threshold, minBlockSize, depth + 1);
        buildNode(node->getBottomLeft(), image, errorMethod, threshold, minBlockSize, depth + 1);
        buildNode(node->getBottomRight(), image, errorMethod, threshold, minBlockSize, depth + 1);
    } else {
        node->setLeaf(true);
    }
}

std::vector<std::vector<Color>> QuadTree::reconstructImage(int panjang, int lebar) {
    //buat gambar sesuai p l
    std::vector<std::vector<Color>> result(lebar, std::vector<Color>(panjang));
    
    //node: root, maka isi warna hasil
    if (root) {
        fillImage(result, root);
    }
    
    return result;
}

void QuadTree::fillImage(std::vector<std::vector<Color>>& image, QuadTreeNode* node) {
    if (!node) return;
    
    if (node->isLeafNode()) {
        //node: leaf, isi warna rata2
        int startX = node->getX();
        int startY = node->getY();
        int endX = std::min(startX + node->getpanjang(), (int)image[0].size());
        int endY = std::min(startY + node->getlebar(), (int)image.size());
        
        for (int y = startY; y < endY; y++) {
            for (int x = startX; x < endX; x++) {
                image[y][x] = node->getAvgColor();
            }
        }
    } else {
        //rekursif tiap anak
        fillImage(image, node->getTopLeft());
        fillImage(image, node->getTopRight());
        fillImage(image, node->getBottomLeft());
        fillImage(image, node->getBottomRight());
    }
}

int QuadTree::hitungCompressedSize() {
    // Compressed Size: total node dikali dengan size per node
    // size per node: posisi (2 int) + ukuran (2 int) + warna (3 byte) + flag (1 byte)
    int sizePerNode = (2 * sizeof(int)) + (2 * sizeof(int)) + (3 * sizeof(char)) + sizeof(bool);
    return totalN * sizePerNode;
}