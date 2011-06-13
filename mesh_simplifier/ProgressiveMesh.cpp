#include "stdafx.h"
#include "ProgressiveMesh.h"
#include <float.h>

ProgressiveMesh::ProgressiveMesh(Mesh *m) {
    _oriMesh = m;
    _mesh = *m;
    _edgeCollapseList.clear();
    _visibleTriangles = _mesh.getNumTriangles();

    // calculate Quadric Matrix();
    set<border> bs;
    for (int i = 0; i < _mesh.getNumVerts(); i++) {
        _mesh.getVertex(i).calcQuadric(_mesh);
        _mesh.getVertex(i).getAllBorderEdges(bs, _mesh);
    }
        // increase borders' error cost
    if (!bs.empty()) {
        set<border>::iterator pos;

        for (pos = bs.begin(); pos != bs.end(); ++pos) {
            border edge = *pos;
            vertex &v1 = _mesh.getVertex(edge.vert1);
            vertex &v2 = _mesh.getVertex(edge.vert2);

            Tuple &vec1 = v1.getXYZ();
            Tuple &vec2 = v2.getXYZ();

            Tuple edgeTuple = vec1 - vec2;
            triangle &tri = _mesh.getTri(edge.triIndex);
            Tuple normal = tri.getNormalTuple();

            Tuple abc = edgeTuple.unitcross(normal);
            float ary[4] = {abc.x, abc.y, abc.z, -(abc.dot(vec1))};

            double q[4][4];
            for (int i = 0; i < 4; i++) 
                for (int j = i; j < 4; j++) 
                    q[i][j] = BORDER_WT * ary[i] * ary[j];
            for (int i = 1; i < 4; i++) 
                for (int j = 0; j < i; j++) 
                    q[i][j] = q[j][i];

            double q1[4][4], q2[4][4];
            v1.getQuadric(q1);
            v2.getQuadric(q2);
            for (int i = 0; i < 4; i++) 
                for (int j = 0; j < 4; j++) 
                    q1[i][j] += q[i][j], q2[i][j] += q[i][j];
            v1.setQuadric(q1);
            v2.setQuadric(q2);
        }
    }

    VertexPointerSet vs;
    vector<VertexPointerSet::iterator> vsVec(_mesh.getNumVerts());

    // calculate edge collapse error cost 
    for (int i = 0; i < _mesh.getNumVerts(); i++) {
        _quadricCollapseCost(_mesh.getVertex(i));
        VertexPointer v;
        v.index = i;
        v.mesh = &_mesh;
        vsVec[i] = vs.insert(v);
    }

    // build edge collapse list
    while(vs.size()) {
        const VertexPointer vp = *(vs.begin());
        vertex v = _mesh.getVertex(vp.index);

        EdgeCollapse e;
        bool valid = true;

        e.from = v.index;
        e.to = v.minCostNeighbor;
        if (-1 == e.to || !_mesh.getVertex(e.from).active) valid = false;
        else if (!_mesh.getVertex(e.to).active) {
            _quadricCollapseCost(v);
            e.to = v.minCostNeighbor;
            if (-1 == e.to || !_mesh.getVertex(e.to).active) valid = false;
        }

        _mesh.getVertex(e.from).active = false;
        vs.erase(vs.begin());

        if (!valid) continue;

        vertex& to = _mesh.getVertex(e.to);
        vertex& from = _mesh.getVertex(e.from);

        // add from's cost to to's
        double qf[4][4], qt[4][4];
        to.getQuadric(qt);
        from.getQuadric(qf);
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) 
                qt[i][j] += qf[i][j];
        to.setQuadric(qt);

        set<int> affectedV;
        _updateTriangles(e, v, affectedV);
        _updateAffectedVerts(vs, vsVec, e, affectedV);
        
        _edgeCollapseList.push_back(e);
    }

    // build over, set all active
    _mesh = *_oriMesh;
    for (int i = 0; i < _mesh.getNumTriangles(); i++) 
        _mesh.getTri(i).active = true;
    _edgeCollapseListIter = _edgeCollapseList.begin();
}

void ProgressiveMesh::_updateAffectedVerts(VertexPointerSet &vs, vector<VertexPointerSet::iterator> &vsVec, const EdgeCollapse &e, set<int> &affectedV) {
    set<int>::iterator it;
    for (it = affectedV.begin(); it != affectedV.end(); ++it) {
        vertex& v = _mesh.getVertex(*it);
        vs.erase(vsVec[*it]);
        vsVec[*it] = vs.end();
        // update affected vertex neighbor 
        if (v.index != e.to) {
            v.addVertNeighbor(e.to);
        } else {
            for (set<int>::const_iterator it2 = affectedV.begin(); it2 != affectedV.end(); ++it2) 
                if (*it2 != e.to) 
                    v.addVertNeighbor(*it2);
        }
        v.removeVertNeighbor(e.from);

        // remove useless vertex
        bool active = false;
        for (set<int>::const_iterator it2 = v.getTriNeighbors().begin(); it2 != v.getTriNeighbors().end(); ++it2) 
            if (_mesh.getTri(*it2).active) {
                active = true;
                break;
            }

        if (active) {
            VertexPointer vp;
            vp.index = v.index;
            vp.mesh = &_mesh;
            vsVec[vp.index] = vs.insert(vp);
            _mesh.getVertex(vp.index).active = true;
            _quadricCollapseCost(_mesh.getVertex(v.index));
        } else 
            _mesh.getVertex(v.index).active = false;
    }
}

void ProgressiveMesh::_updateTriangles(EdgeCollapse &e, vertex &v, set<int> &affectedV) {
    set<int> & triangleNeighbors = v.getTriNeighbors();
    set<int>::iterator it;
    for (it = triangleNeighbors.begin(); it != triangleNeighbors.end(); ++it) {
        int triIndex = *it;
        triangle &t = _mesh.getTri(triIndex);
        if (!t.active) continue;
        bool remove = false;
        if (t.hasVertex(e.from) && t.hasVertex(e.to)) {
            e.removed.insert(triIndex);
            t.changeVertex(e.from, e.to);
            remove = true;
            t.active = false;
        } else {
            t.changeVertex(e.from, e.to);
            t.calcNormal();
            _mesh.getVertex(e.to).addTriNeighbor(triIndex);
            e.affected.insert(triIndex);
        }
        int vert[3];
        t.getVerts(vert[0], vert[1], vert[2]);
        for (int i = 0; i < 3; i++) {
            affectedV.insert(vert[i]);
            if (remove) 
                _mesh.getVertex(vert[i]).removeTriNeighbor(triIndex);
        }
    }
}

void ProgressiveMesh::_quadricCollapseCost(vertex& v) {
    double mincost = FLT_MAX;
    double q1[4][4];
    v.getQuadric(q1);
    set<int> &neighbors = v.getVertNeighbors();
    set<int>::iterator it;
    for (it = neighbors.begin(); it != neighbors.end(); ++it) {
        vertex& n = _mesh.getVertex(*it);
        if (!n.active) continue;
        if (n == v) continue;
        double q2[4][4];
        double qs[4][4];

        n.getQuadric(q2);

        for (int i = 0; i < 4; i++) 
            for (int j = 0; j < 4; j++) 
                qs[i][j] = q1[i][j] + q2[i][j];

        double cost = _calcQuadricError(qs, n);
        if (cost < mincost) {
            mincost = cost;
            v.cost = cost;
            v.minCostNeighbor = *it;
        }
    }
}

// calculate v'Qv
double ProgressiveMesh::_calcQuadricError(double Q[4][4], vertex& v) {
    double tmp[4];
    const Tuple t = v.getXYZ();
    for (int i = 0; i < 4; i++) 
        tmp[i] = t.x * Q[0][i] + t.y * Q[1][i] + t.z * Q[2][i] + Q[3][i];
    return tmp[0] * t.x + tmp[1] * t.y + tmp[2] * t.z + tmp[3];
}

bool ProgressiveMesh::simplify(int number) {
    int flag = 1;
    if (number < 0) {
        flag = -1;
        number = -number;
    }
    while(number--) {
        if ( 1 == flag && _edgeCollapseListIter == _edgeCollapseList.end() ||
            -1 == flag && _edgeCollapseListIter == _edgeCollapseList.begin())
            return false;
        if (flag < 0) --_edgeCollapseListIter;
        EdgeCollapse &e = *_edgeCollapseListIter;
        set<int>::iterator it;

        for (it = e.removed.begin(); it != e.removed.end(); ++it) {
            int index = *it;
            triangle &t = _mesh.getTri(index);
            t.active = (flag < 0);
        }

        for (it = e.affected.begin(); it != e.affected.end(); ++it) {
            int index = *it;
            triangle &t = _mesh.getTri(index);
            if (flag > 0) t.changeVertex(e.from, e.to);
            else t.changeVertex(e.to, e.from);
            t.calcNormal();
        }
        if (flag > 0) 
            ++_edgeCollapseListIter;
        _visibleTriangles -= e.removed.size() * flag;
    }
    return true;
}

