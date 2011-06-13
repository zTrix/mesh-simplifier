#pragma once
#include <list>
#include <vector>
#include "mesh.h"

#define BORDER_WT 1000

struct EdgeCollapse {
    int from;
    int to;
    set<int> removed;
    set<int> affected;
};

struct VertexPointer {
    Mesh * mesh;
    int index;

    bool operator<(const VertexPointer& vp) const {
        return (mesh->getVertex(index) < vp.mesh->getVertex(vp.index));
    }
};

typedef multiset<VertexPointer, less<VertexPointer> > VertexPointerSet;

class ProgressiveMesh {
public:
    ProgressiveMesh(Mesh *mesh);

    bool simplify(int number);

    int triangleNumber() {
        return _mesh.getNumTriangles();
    }
    int visibleTriangleNumber(){
        return _visibleTriangles;
    }
    triangle& getTriangle(int i) {
        return _mesh.getTri(i);
    }
    int edgeCollapseNumber() {
        return _edgeCollapseList.size();
    }

private:
    Mesh * _oriMesh;
    Mesh _mesh;
    list<EdgeCollapse> _edgeCollapseList;
    list<EdgeCollapse>::iterator _edgeCollapseListIter;
    unsigned int _visibleTriangles;

    void _updateTriangles(EdgeCollapse &e, vertex &v, set<int> &affectedV);
    void _updateAffectedVerts(VertexPointerSet &vs, vector<VertexPointerSet::iterator> &vsVec, const EdgeCollapse &e, set<int> &affectedV);
    void _quadricCollapseCost(vertex& v);
    double _calcQuadricError(double Q[4][4], vertex& v);

};

