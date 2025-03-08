#ifndef Vec2_H
#define Vec2_H

#include <cmath>
#include <cstdlib>
#include <iostream>

class Vec2;
static inline Vec2 operator + (Vec2 const & a , Vec2 const & b);
static inline Vec2 operator * (float a , Vec2 const & b);

class Vec2 {
private:
    float mVals[2];
public:
    Vec2() {mVals[0] = mVals[1] = 0.f;}
    Vec2( float x , float y ) {
       mVals[0] = x; mVals[1] = y;
    }
    float & operator [] (unsigned int c) { return mVals[c]; }
    float operator [] (unsigned int c) const { return mVals[c]; }
    Vec2 operator = (Vec2 const & other) {
       mVals[0] = other[0] ; mVals[1] = other[1];
       return *this;
    }
    float squareLength() const {
       return mVals[0]*mVals[0] + mVals[1]*mVals[1];
    }
    float length() const { return sqrt( squareLength() ); }
    inline
    float norm() const { return length(); }
    inline
    float squareNorm() const { return squareLength(); }
    void normalize() { float L = length(); mVals[0] /= L; mVals[1] /= L; mVals[2] /= L; }
    static float dot( Vec2 const & a , Vec2 const & b ) {
       return a[0]*b[0] + a[1]*b[1];
    }
    void operator += (Vec2 const & other) {
        mVals[0] += other[0];
        mVals[1] += other[1];
    }
    void operator -= (Vec2 const & other) {
        mVals[0] -= other[0];
        mVals[1] -= other[1];
    }
    void operator *= (Vec2 const & other) {
        mVals[0] *= other[0];
        mVals[1] *= other[1];
    }
    void operator *= (float s) {
        mVals[0] *= s;
        mVals[1] *= s;
    }
    void operator /= (float s) {
        mVals[0] /= s;
        mVals[1] /= s;
    }
    static Vec2 compProduct(Vec2 const & a , Vec2 const & b) {
        return Vec2(a[0]*b[0] , a[1]*b[1]);
    }
};

static inline Vec2 operator + (Vec2 const & a , Vec2 const & b) {
   return Vec2(a[0]+b[0] , a[1]+b[1]);
}
static inline Vec2 operator - (Vec2 const & a , Vec2 const & b) {
   return Vec2(a[0]-b[0] , a[1]-b[1]);
}
static inline Vec2 operator * (float a , Vec2 const & b) {
   return Vec2(a*b[0] , a*b[1]);
}
static inline Vec2 operator * (Vec2 const & a , Vec2 const & b) {
   return Vec2(a[0]*b[0] , a[1]*b[1]);
}
static inline Vec2 operator * (Vec2 const & b , float a ) {
   return Vec2(a*b[0] , a*b[1]);
}
static inline Vec2 operator / (Vec2 const &  a , float b) {
   return Vec2(a[0]/b , a[1]/b );
}
static inline std::ostream & operator << (std::ostream & s , Vec2 const & p) {
    s << p[0] << " " << p[1];
    return s;
}
static inline std::istream & operator >> (std::istream & s , Vec2 & p) {
    s >> p[0] >> p[1];
    return s;
}

#endif
