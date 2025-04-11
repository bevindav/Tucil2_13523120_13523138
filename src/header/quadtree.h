#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <string>

struct Color {
    unsigned char r, g, b;
    
    Color() : r(0), g(0), b(0) {}
    Color(unsigned char red, unsigned char green, unsigned char blue) : r(red), g(green), b(blue) {}
};

class QuadTreeNode {
private:
    int x, y;            
    int lebar, panjang;   
    Color avgColor;      
    double error;
    bool isLeaf;         
    QuadTreeNode* topLeft;
    QuadTreeNode* topRight;
    QuadTreeNode* bottomLeft;
    QuadTreeNode* bottomRight;

public:
    // ctor
    QuadTreeNode(int x, int y, int lebar, int panjang);
    
    // dtor
    ~QuadTreeNode();
    
    int getX() const { return x; }
    int getY() const { return y; }
    int getlebar() const { return lebar; }
    int getpanjang() const { return panjang; }
    Color getAvgColor() const { return avgColor; }
    bool isLeafNode() const { return isLeaf; }
    
    void setAvgColor(const Color& color) { avgColor = color; }
    void setLeaf(bool leaf) { isLeaf = leaf; }
    
    QuadTreeNode* getTopLeft() const { return topLeft; }
    QuadTreeNode* getTopRight() const { return topRight; }
    QuadTreeNode* getBottomLeft() const { return bottomLeft; }
    QuadTreeNode* getBottomRight() const { return bottomRight; }
    
    void split();
    
    bool hasChildren() const;

    void setError(double err);
    double getError() const;
};

class QuadTree {
private:
    QuadTreeNode* root;
    int totalN;     
    int maxDepth;
    int realWidth;
    int realHeight;
    
public:
    // ctor
    QuadTree();
    
    // dtor
    ~QuadTree();
    std::vector<std::vector<Color>> reconstructImage(int lebar, int panjang);
    
    QuadTreeNode* getRoot() const { return root; }
    int getTotalNodes() const { return totalN; }
    int getMaxDepth() const { return maxDepth; }

    void buildfrImage(const std::vector<std::vector<Color>>& image, int errorMethod, double threshold, int minBlockSize);
    
    void buildNode(QuadTreeNode* node, const std::vector<std::vector<Color>>& image, int errorMethod, double threshold, int minBlockSize, int depth);
        
    void fillImage(std::vector<std::vector<Color>>& image, QuadTreeNode* node);
    int hitungCompressedSize();
    std::vector<std::vector<Color>> reconstructImageForGIF(int depth);
    void fillImageLimited(std::vector<std::vector<Color>>& image, QuadTreeNode* node, int maxDepth, int currentDepth);
};

#endif