#pragma once 

#include <math.h>
#include <iostream>

class Tuple{
public:
    float x, y, z;

    // Constructors and Destructors
    Tuple() {x = y = z = 0.0f;};
    Tuple(float x1, float y1, float z1) {x = x1; y = y1; z = z1;};
    Tuple(float av[3]) {x = av[0]; y = av[1]; z = av[2];};
    Tuple(const Tuple& v) {x = v.x; y = v.y; z = v.z;};
    ~Tuple() {}; // Destructor intentially does nothing

    // Assignment operator
    Tuple& operator=(const Tuple& v) {x = v.x; y = v.y; z = v.z; return *this;};

    // Comparision operators
    bool operator==(const Tuple& v) {return (x == v.x && y == v.y && z == v.z);};
    bool operator!=(const Tuple& v) {return (x != v.x || y != v.y || z != v.z);};

    // Scalar operations
    Tuple operator+(float f) const {return Tuple(x + f, y + f, z + f);};
    Tuple operator-(float f) const {return Tuple(x - f, y - f, z - f);};
    Tuple operator*(float f) const {return Tuple(x * f, y * f, z * f);};
    Tuple operator/(float f) const {Tuple v1(x,y,z); if (f != 0.0f) {v1.x /= f; v1.y /= f; v1.z /= f;}; return v1;};

    Tuple& operator+=(float f) {x += f; y += f; z += f; return *this;};
    Tuple& operator-=(float f) {x -= f; y -= f; z -= f; return *this;};
    Tuple& operator*=(float f) {x *= f; y *= f; z *= f; return *this;};
    Tuple& operator/=(float f) {if(f!=0.0f){ x /= f; y /= f; z /= f;}; return *this;};


    // Vector operations
    Tuple operator+(const Tuple& v) const {return Tuple(x + v.x, y + v.y, z + v.z);};
    Tuple& operator+=(const Tuple& v) {x += v.x; y += v.y; z += v.z; return *this;};
    Tuple operator-(const Tuple& v) const {return Tuple(x - v.x, y - v.y, z - v.z);};
    Tuple& operator-=(const Tuple& v) {x -= v.x; y -= v.y; z -= v.z; return *this;};

    // Unary operators
    Tuple operator-() const {return Tuple (-x, -y, -z); };

    // Dot and Cross Products
    float dot(const Tuple& v) const {return (x * v.x + y * v.y + z * v.z);};
    Tuple cross(const Tuple& v) const {return Tuple(y * v.z - z * v.y,
                                             z * v.x - x * v.z,
                                             x * v.y - y * v.x);};
    Tuple unitcross(const Tuple& v) const {Tuple vr(y * v.z - z * v.y,
                                             z * v.x - x * v.z,
                                             x * v.y - y * v.x); vr.normalize(); return vr;};

    // Miscellaneous
    void normalize() {float a = float(sqrt(x*x + y*y + z*z)); if (a!=0.0f) {x/=a; y/=a; z/=a;};};
    void setZero() {x = y = z = 0.0f;};
    float length() {return float(sqrt(x*x + y*y + z*z));};

    // Friend functions
    friend Tuple operator*(float a, const Tuple& v) {return Tuple (a * v.x, a * v.y, a * v.z);};

    // dot and cross products
    float dot(const Tuple& v1, const Tuple& v2) {return (v1.x * v2.x + v1.y * v2.y +v1. z * v2.z);};
    Tuple cross(const Tuple& v1, const Tuple& v2)  {return Tuple (v1.y * v2.z - v1.z * v2.y,
                                                        v1.z * v2.x - v1.x * v2.z,
                                                            v1.x * v2.y - v1.y * v2.x);};
    Tuple unitcross(const Tuple& v1, const Tuple& v2)  {Tuple vr(v1.y * v2.z - v1.z * v2.y,
                                                    v1.z * v2.x - v1.x * v2.z,
                                                    v1.x * v2.y - v1.y * v2.x); 
                                                    vr.normalize(); return vr;};


    // Input and Output
    friend std::ostream& operator<<(std::ostream& os, const Tuple& vo);

};
