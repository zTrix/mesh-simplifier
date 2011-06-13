#include "stdafx.h"
#include <assert.h>
#include <float.h>
#include <iostream>
#include <stdio.h>
#include "Mesh.h"

Mesh::Mesh(char* filename) {
    _numVerts = _numTriangles = 0;
    if (!loadObjFile(filename)) {
        MessageBox(NULL, "Failed to load obj file"__FILE__, NULL, MB_OK);
        _numVerts = _numTriangles = 0;
        _vlist.clear();
        _plist.clear();
    }
}

Mesh::Mesh(const Mesh& m) {
    _numVerts = m._numVerts;
    _numTriangles = m._numTriangles;
    _vlist = m._vlist; 
    _plist = m._plist;
}

Mesh& Mesh::operator=(const Mesh& m) {
    if (this == &m) return *this;
    _numVerts = m._numVerts;
    _numTriangles = m._numTriangles;
    _vlist = m._vlist; 
    _plist = m._plist;
    return *this;
}

Mesh::~Mesh() {
    _numVerts = _numTriangles = 0;
    _vlist.erase(_vlist.begin(), _vlist.end());
    _plist.erase(_plist.begin(), _plist.end());
}

bool Mesh::loadObjFile(char* filename) {
    FILE* inFile = NULL;
    fopen_s(&inFile, filename, "r");
    if (inFile == NULL) {
        char pszError[_MAX_FNAME + 1];
        sprintf_s(pszError, _MAX_FNAME, "%s does not exist!\n", filename);
        MessageBox(NULL, pszError, NULL, MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    int rd = 0, vindex = 0, findex = 0, c;
    char tmp[1024];
    int size = sizeof(tmp);
    do {
        rd = fscanf_s(inFile, "%s", tmp);
        if (rd == EOF || rd <= 0) break;
        if (tmp[0] == '#') {
            do {
                c = fgetc(inFile);
            } while(c != '\n' && c != EOF);
        } else if (tmp[0] == 'g') {
            do {
                c = fgetc(inFile);
            } while(c != '\n' && c != EOF);
        } else if (tmp[0] == 'v') {
            float x, y, z;
            fscanf_s(inFile, "%f", &x);
            fscanf_s(inFile, "%f", &y);
            fscanf_s(inFile, "%f", &z);
            cout << x << y << z;
            vertex v(x, y, z);
            v.index = vindex++;
            _vlist.push_back(v);

            do {
                c = fgetc(inFile);
            } while(c != '\n' && c != EOF);
        } else if (tmp[0] == 'f') {
            unsigned int v1, v2, v3;
            fscanf_s(inFile, "%d", &v1);
            fscanf_s(inFile, "%d", &v2);
            fscanf_s(inFile, "%d", &v3);
            v1--;
            v2--;
            v3--;
            assert(v1 < vindex && v2 < vindex && v3 < vindex);

            triangle t(this, v1, v2, v3);
            t.index = findex;

            _plist.push_back(t);

            _vlist[v1].addTriNeighbor(findex);
            _vlist[v1].addVertNeighbor(v2);
            _vlist[v1].addVertNeighbor(v3);

            _vlist[v2].addTriNeighbor(findex);
            _vlist[v2].addVertNeighbor(v1);
            _vlist[v2].addVertNeighbor(v3);

            _vlist[v3].addTriNeighbor(findex);
            _vlist[v3].addVertNeighbor(v1);
            _vlist[v3].addVertNeighbor(v2);
            
            findex++;
            do {
                c = fgetc(inFile);
            } while(c != '\n' && c != EOF);
        } else if (tmp[0] != '\n'){
            do {
                c = fgetc(inFile);
            } while(c != '\n' && c != EOF);
        }
    } while(true);

    fclose(inFile); // close the file
    _numVerts = vindex;
    _numTriangles = findex;

    char msg[1024];
    sprintf_s(msg, 1024, "vertex: %d\nfaces: %d\n", vindex, findex);
    MessageBox(NULL, msg, NULL, MB_OK | MB_ICONINFORMATION);

    return true;
}

void Mesh::setMinMax(float min[3], float max[3]) {
    max[0] = max[1] = max[2] = -FLT_MAX;
    min[0] = min[1] = min[2] = FLT_MAX;

    for (unsigned int i = 0; i < _vlist.size(); ++i) {
        const float* pVert = _vlist[i].getArrayVerts();
        if (pVert[0] < min[0]) min[0] = pVert[0];
        if (pVert[1] < min[1]) min[1] = pVert[1];
        if (pVert[2] < min[2]) min[2] = pVert[2];
        if (pVert[0] > max[0]) max[0] = pVert[0];
        if (pVert[1] > max[1]) max[1] = pVert[1];
        if (pVert[2] > max[2]) max[2] = pVert[2];
    }
}

void Mesh::Normalize() {
    float min[3], max[3], Scale;

    setMinMax(min, max);

    Tuple minv(min);
    Tuple maxv(max);

    Tuple dimv = maxv - minv;
    
    if (dimv.x >= dimv.y && dimv.x >= dimv.z) Scale = 2.0f/dimv.x;
    else if (dimv.y >= dimv.x && dimv.y >= dimv.z) Scale = 2.0f/dimv.y;
    else Scale = 2.0f/dimv.z;

    Tuple transv = minv + maxv;

    transv *= 0.5f;

    for (unsigned int i = 0; i < _vlist.size(); ++i) {
        _vlist[i].getXYZ() -= transv;
        _vlist[i].getXYZ() *= Scale;
    }
}

