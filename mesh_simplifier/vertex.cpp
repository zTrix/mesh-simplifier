#include "stdafx.h"
#include "vertex.h"
#include "mesh.h"

using namespace std;

// Calculate the Quadric 4x4 matrix
void vertex::calcQuadric(Mesh& m) {
    for (int i = 0; i < 4; ++i) 
        for (int j = i; j < 4; ++j) 
            _Q[i][j] = 0;

    set<int>::iterator pos;
    for (pos = _triNeighbors.begin(); pos != _triNeighbors.end(); ++pos) {
        int triIndex = *pos;
        triangle& t = m.getTri(triIndex);
        if (t.active) {

            const Tuple normal = t.getNormalTuple();
            float ary[] = {normal.x, normal.y, normal.z, t.getD()};

            for (int i = 0; i < 4; i++)
                for (int j = i; j < 4; j++)
                    _Q[i][j] += ary[i] * ary[j];
        }
    }
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < i; j++)
            _Q[i][j] = _Q[j][i];
}

void vertex::getAllBorderEdges(set<border> &borderSet, Mesh& m) {
    set<int>::iterator pos, pos2;

    for (pos = getVertNeighbors().begin(); pos != getVertNeighbors().end(); ++pos) {
        int triCount = 0;
        int triIndex = -1;
        vertex& v = m.getVertex(*pos);
        for (pos2 = v.getTriNeighbors().begin(); pos2 != v.getTriNeighbors().end(); ++pos2) {
            if (m.getTri(*pos2).hasVertex(index) ) {
                ++triCount;
                triIndex = m.getTri(*pos2).index;
            }
        }

        if (1 == triCount) {
            // store the smaller index first
            border b;
            b.triIndex = triIndex;
            if (index < v.index) {
                b.vert1 = index;
                b.vert2 = v.index;
            } else {
                b.vert1 = v.index;
                b.vert2 = index;
            }
            borderSet.insert(b);
        }
    }
}

