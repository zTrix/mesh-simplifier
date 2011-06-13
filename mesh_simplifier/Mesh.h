#pragma once

#include <windows.h>
#include <vector>
#include "vertex.h"
#include "triangle.h"

class Mesh {
public:
    Mesh() {_numVerts = _numTriangles = 0;};
    Mesh(char* filename);
    ~Mesh();

    Mesh(const Mesh&); 
    Mesh& operator=(const Mesh&); 

    vertex& getVertex(int index) {return _vlist[index];};
    const vertex& getVertex(int index) const {return _vlist[index];};
    triangle& getTri(int index) {return _plist[index];};
    const triangle& getTri(int index) const {return _plist[index];};

    int getNumVerts() {return _numVerts;};
    void setNumVerts(int n) {_numVerts = n;};
    int getNumTriangles() {return _numTriangles;};
    void setNumTriangles(int n) {_numTriangles = n;};

    void Normalize();

private:
    // vertex
    vector<vertex> _vlist; 
    // triangles
    vector<triangle> _plist; 

    int _numVerts, _numTriangles;

    bool loadObjFile(char* filename); // load from PLY file

    // get bounding box for mesh
    void setMinMax(float min[3], float max[3]);
};

