#pragma once

#include <iostream>
#include "tuple.h"

class vertex;
class Mesh;

class triangle {
public:
    bool active;
    int index;

    triangle(Mesh* mp, int v1, int v2, int v3) : 
        _vert1(v1), _vert2(v2), _vert3(v3), _mesh(mp), active(true),
        index(-1) {
        calcNormal();
    };

    triangle(const triangle& t) :  
        _vert1(t._vert1), _vert2(t._vert2), _vert3(t._vert3), 
        _mesh(t._mesh), active(t.active),
        index(t.index) {
        calcNormal();
    };

    triangle& operator=(const triangle& t) {
        if (&t == this) return *this; // check for assignment to self
        _vert1 = t._vert1;
        _vert2 = t._vert2;
        _vert3 = t._vert3;
        _mesh = t._mesh;
        _normal = t._normal;
        active = t.active;
        index = t.index;
        return *this;
    }

    bool operator==(const triangle& t) {
        return (_vert1 == t._vert1 && 
                _vert2 == t._vert2 && 
                _vert3 == t._vert3 &&
                _mesh == t._mesh
        );
    }

    void calcNormal();

    bool hasVertex(int vi) {
        return  (vi == _vert1 || vi == _vert2 || vi == _vert3);
    }

    void changeVertex(int vFrom, int vTo) {
        if (vFrom == _vert1) 
            _vert1 = vTo;
        else if (vFrom == _vert2) 
            _vert2 = vTo;
        else if (vFrom == _vert3) 
            _vert3 = vTo;
    }

    void getVerts(int& v1, int& v2, int& v3) {v1=_vert1;v2=_vert2;v3=_vert3;}

    const float* getVert1();
    const float* getVert2();
    const float* getVert3();

    float* getNormal() {_normArray[0]=_normal.x;
                        _normArray[1]=_normal.y;
                        _normArray[2]=_normal.z;
                        return _normArray;}
    const Tuple& getNormalTuple() const {return _normal;}

    float getD() const {return _d;}

protected:
    int _vert1, _vert2, _vert3;
    Tuple _normal; // normal to plane
    float _normArray[3], _d;
    Mesh* _mesh;
};

