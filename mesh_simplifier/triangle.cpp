#include "stdafx.h"
#include "triangle.h"
#include "mesh.h"

const float* triangle::getVert1() {return (_mesh->getVertex(_vert1)).getArrayVerts();}
const float* triangle::getVert2() {return (_mesh->getVertex(_vert2)).getArrayVerts();}
const float* triangle::getVert3() {return (_mesh->getVertex(_vert3)).getArrayVerts();}

void triangle::calcNormal() {
    Tuple vec1 = (_mesh->getVertex(_vert1)).getXYZ();
    Tuple vec2 = (_mesh->getVertex(_vert2)).getXYZ();
    Tuple vec3 = (_mesh->getVertex(_vert3)).getXYZ();
    _normal = (vec2 - vec1).unitcross(vec3 - vec2);
    _d = -_normal.dot(vec1);
}

