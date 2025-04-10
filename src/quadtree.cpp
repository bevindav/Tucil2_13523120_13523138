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
    int halfPanjang = panjang / 2;
    int halfLebar = lebar / 2;
    
    topLeft = new QuadTreeNode(x, y, halfPanjang, halfLebar);
    topRight = new QuadTreeNode(x + halfPanjang, y, panjang - halfPanjang, halfLebar);
    bottomLeft = new QuadTreeNode(x, y + halfLebar, halfPanjang, lebar - halfLebar);
    bottomRight = new QuadTreeNode(x + halfPanjang, y + halfLebar, panjang - halfPanjang, lebar - halfLebar);
    
    isLeaf = false;
}

bool QuadTreeNode::hasChildren() const {
    return !isLeaf && topLeft != nullptr;
}

QuadTree::QuadTree() : root(nullptr), totalN(0), maxDepth(0) {}

QuadTree::~QuadTree() {
    if (root) delete root;
}

void QuadTree::buildfrImage(const std::vector<std::vector<Color>>& image, int errorMethod, double errorThreshold, int minBlockSize) {
    if (image.empty() || image[0].empty()) return;
    int panjang = image[0].size();
    int lebar = image.size();
    
    realWidth = panjang;
    realHeight = lebar;

    root = new QuadTreeNode(0, 0, panjang, lebar);
    totalN = 1;
    // this->maxDepth = 0;
    
    buildNode(root, image, errorMethod, errorThreshold, minBlockSize, 0);
}

void QuadTree::buildNode(QuadTreeNode* node, const std::vector<std::vector<Color>>& image, int errorMethod, double errorThreshold, int minBlockSize, int currentDepth) {
    if (!node) return;
        
    this->maxDepth = std::max(this->maxDepth, currentDepth);
        
    int nodeX = node->getX();
    int nodeY = node->getY();
    int nodePanjang = node->getpanjang();
    int nodeLebar = node->getlebar();
        
    std::vector<Color> blockPixels;
    for (int y = nodeY; y < nodeY + nodeLebar; ++y) {
        for (int x = nodeX; x < nodeX + nodePanjang; ++x) {
            if (y < image.size() && x < image[0].size())
                blockPixels.push_back(image[y][x]);
        }
    }
        
    Color avgColor = hitungAverageColor(blockPixels);
    node->setAvgColor(avgColor);

    double error = 0.0;
    switch (errorMethod) {
        case 1: error = hitungVariance(blockPixels, avgColor); break;
        case 2: error = hitungMAD(blockPixels, avgColor); break;
        case 3: error = hitungMaxDifference(blockPixels); break;
        case 4: error = hitungEntropy(blockPixels); break;
        case 5: error = hitungSSIM(blockPixels, avgColor); break;
        default: error = hitungVariance(blockPixels, avgColor); break;
    }
    node->setError(error);
    
    // Ini pengecekan minBlockSize-nya
    if (nodePanjang <= minBlockSize || nodeLebar <= minBlockSize) {
        node->setLeaf(true);
        return;
    }
    
    if ((errorMethod == 5 && error >= errorThreshold) ||
        (errorMethod != 5 && error <= errorThreshold)) {
        node->setLeaf(true);
        return;
    }
    
    node->split();
    totalN += 4;
    
    buildNode(node->getTopLeft(), image, errorMethod, errorThreshold, minBlockSize, currentDepth + 1);
    buildNode(node->getTopRight(), image, errorMethod, errorThreshold, minBlockSize, currentDepth + 1);
    buildNode(node->getBottomLeft(), image, errorMethod, errorThreshold, minBlockSize, currentDepth + 1);
    buildNode(node->getBottomRight(), image, errorMethod, errorThreshold, minBlockSize, currentDepth + 1);
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

std::vector<std::vector<Color>> QuadTree::reconstructImageLimitedDepth(int maxDepth) {
    std::vector<std::vector<Color>> image(realHeight, std::vector<Color>(realWidth));
    reconstructLimitedHelper(root, image, 0, 0, realWidth, realHeight, 0, maxDepth);
    return image;
}


void QuadTree::reconstructLimitedHelper(
    QuadTreeNode* node,
    std::vector<std::vector<Color>>& image,
    int x, int y, int width, int height,
    int currentDepth, int maxDepth
) {
    if (!node) return;

    int endX = std::min(x + width, (int)image[0].size());
    int endY = std::min(y + height, (int)image.size());
    
    if (node->isLeafNode() || currentDepth >= maxDepth) {
        Color avgColor = node->getAvgColor();
        for (int i = y; i < endY; ++i) {
            for (int j = x; j < endX; ++j) {
                image[i][j] = avgColor;
            }
        }
        return;
    }
    
    int halfWidth = width / 2;
    int halfHeight = height / 2;
    
    reconstructLimitedHelper(node->getTopLeft(), image, x, y, halfWidth, halfHeight, currentDepth + 1, maxDepth);
    reconstructLimitedHelper(node->getTopRight(), image, x + halfWidth, y, halfWidth, halfHeight, currentDepth + 1, maxDepth);
    reconstructLimitedHelper(node->getBottomLeft(), image, x, y + halfHeight, halfWidth, halfHeight, currentDepth + 1, maxDepth);
    reconstructLimitedHelper(node->getBottomRight(), image, x + halfWidth, y + halfHeight, halfWidth, halfHeight, currentDepth + 1, maxDepth);
}

std::vector<std::vector<Color>> QuadTree::reconstructImageForGIF(int depth) {
    std::vector<std::vector<Color>> result(realHeight, std::vector<Color>(realWidth));
    Color bgColor = root->getAvgColor();
    for (int y = 0; y < realHeight; y++) {
        for (int x = 0; x < realWidth; x++) {
            result[y][x] = bgColor;
        }
    }
    fillImageLimited(result, root, depth, 0);
    
    return result;
}

void QuadTree::fillImageLimited(std::vector<std::vector<Color>>& image, QuadTreeNode* node, int maxDepth, int currentDepth) {
    if (!node) return;
        if (node->isLeafNode() || currentDepth >= maxDepth) {
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
        fillImageLimited(image, node->getTopLeft(), maxDepth, currentDepth + 1);
        fillImageLimited(image, node->getTopRight(), maxDepth, currentDepth + 1);
        fillImageLimited(image, node->getBottomLeft(), maxDepth, currentDepth + 1);
        fillImageLimited(image, node->getBottomRight(), maxDepth, currentDepth + 1);
    }
}

void QuadTreeNode::setError(double err) {
    error = err;
  }
  
double QuadTreeNode::getError() const {
    return error;
}