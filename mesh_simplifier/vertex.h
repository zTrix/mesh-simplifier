#pragma once
#include <windows.h>

#include <vector>
#include <set>

using namespace std;

#include "tuple.h"
#include "triangle.h"

class Mesh;

struct border {
    int vert1;
    int vert2;
    int triIndex;

    bool operator<(const border& b) const { // for STL set to determine the equality
        int v1, v2, b1, b2;
        // make sure the smaller vert index is always first.
        if (vert1 < vert2) {
            v1 = vert1; v2 = vert2;
        } else {
            v1 = vert2; v2 = vert1;
        }
        if (b.vert1 < b.vert2) {
            b1 = b.vert1; b2 = b.vert2;
        } else {
            b1 = b.vert2; b2 = b.vert1;
        }
        if (v1 < b1) return true;
        if (v1 > b1) return false;
        return (v2 < b2); // v1 == b1
    }
};

class vertex {
public:
    bool active;
    double cost;
    int minCostNeighbor, index;

    vertex(float x1, float y1, float z1) : 
        _myVertex(x1, y1, z1), 
        active(true), cost(0), minCostNeighbor(-1),
        index(-1){
        initQuadric();
    };

    // copy constructor
    vertex(const vertex& v) : _myVertex(v._myVertex), 
                            _vertNeighbors(v._vertNeighbors), _triNeighbors(v._triNeighbors),
                            active(v.active), cost(v.cost), 
                            minCostNeighbor(v.minCostNeighbor),
                            index(v.index) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                _Q[i][j] = v._Q[i][j];
            }
        }       
    };

    ~vertex() {_vertNeighbors.erase(_vertNeighbors.begin(), _vertNeighbors.end());
                _triNeighbors.erase(_triNeighbors.begin(), _triNeighbors.end());};

    vertex& operator=(const vertex& v) {
        if (this == &v) return *this; // check for assignment to self
        _myVertex =v._myVertex;
        _vertNeighbors = v._vertNeighbors;
        _triNeighbors = v._triNeighbors;
        active = v.active;
        cost = v.cost;
        minCostNeighbor = v.minCostNeighbor;
        index = v.index;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                _Q[i][j] = v._Q[i][j];
            }
        }
        return *this;
    };

    bool operator==(const vertex& v) {return (_myVertex == v._myVertex);};
    bool operator!=(const vertex& v) {return (_myVertex != v._myVertex);};

    const float* getArrayVerts() const {
        _v[0]=_myVertex.x;
        _v[1]=_myVertex.y;
        _v[2]=_myVertex.z;
        return _v;
    }

    void addVertNeighbor(int v) {
        _vertNeighbors.insert(v);
    }

    unsigned removeVertNeighbor(int v) {
        return _vertNeighbors.erase(v);
    }

    void addTriNeighbor(int t) {
        _triNeighbors.insert(t);
    }

    unsigned  removeTriNeighbor(int t) {
        return _triNeighbors.erase(t);
    }

    Tuple& getXYZ() {return _myVertex;};
    const Tuple& getXYZ() const {return _myVertex;};

    const set<int>& getVertNeighbors() const {return _vertNeighbors;}
    set<int>& getVertNeighbors() {return _vertNeighbors;}
    const set<int>& getTriNeighbors() const {return _triNeighbors;}
    set<int>& getTriNeighbors() {return _triNeighbors;}

    bool hasVertNeighbor(int v) const {return (_vertNeighbors.find(v) != _vertNeighbors.end());}
    bool hasTriNeighbor(int t) const {return (_triNeighbors.find(t) != _triNeighbors.end());}

    // used to order vertices by edge removal costs
    bool operator<(const vertex& v) const {return (cost < v.cost);}
    bool operator>(const vertex& v) const {return (cost > v.cost);}

    // Used for Garland & Heckbert's quadric edge collapse cost (used for mesh simplifications/progressive meshes)
    void calcQuadric(Mesh& m); // calculate the 4x4 Quadric matrix

    void getQuadric(double Qret[4][4]) {
        for (int i = 0; i < 4; ++i) 
            for (int j = 0; j < 4; ++j) 
                Qret[i][j] = _Q[i][j];
    }

    void setQuadric(double Qnew[4][4]) {
        for (int i = 0; i < 4; ++i) 
            for (int j = 0; j < 4; ++j) 
                 _Q[i][j] = Qnew[i][j];
    }

    void getAllBorderEdges(set<border> &borderSet, Mesh& m);

private:
    Tuple _myVertex; // X, Y, Z position of this vertex

    set<int> _vertNeighbors; // connected to this vertex via an edge
    set<int> _triNeighbors; // triangles of which this vertex is a part


    mutable float _v[3];

    double _Q[4][4]; // Used for Quadric error cost.

    void initQuadric() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                _Q[i][j] = -1;
    }
};

