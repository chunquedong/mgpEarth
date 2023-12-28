/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef COMMON_H
#define COMMON_H

//#include "cppfan/cppfan.h"

#define PF_USING_NAMESPACE using namespace mgp;

#define FE_BEGIN_NAMESPACE namespace fastEarth {
#define FE_END_NAMESPACE }
#define FE_USING_NAMESPACE using namespace fastEarth;


/**
 * suppress unused warning
 */
#define CF_UNUSED(p) ((void)p)

/*========================================================================
 * Field
 */
#define CF_FIELD(Type, name) private: Type _##name;\
  public: Type name() const { return _##name; }\
  public: void name(Type name_) { _##name = name_; }\
  private:

#define CF_READONLY_FIELD(Type, name) private: Type _##name;\
  public: Type name() const { return _##name; }\
  private:

#define CF_FIELD_POINTER(Type, name) private: Type _##name;\
  public: Type *name() { return &_##name; }\
  private:

#define CF_FIELD_REF(Type, name) private: Type _##name;\
  public: Type &name() { return _##name; }\
  public: void name(Type &name_) { _##name = name_; }\
  private:

#define CF_FIELD_CONST_REF(Type, name) private: Type _##name;\
  public: const Type &name() const { return _##name; } \
  public: void name(Type &name_) { _##name = name_; }\
  private:

/////////////////////////////////////////////////////////////////////////
#include <math.h>

#define cf_tolerance 1e-8

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

namespace Math {

    double const PI = 3.14159265358979323846;

    template<typename T>
    T max(T a, T b) { return (a > b) ? a : b; }

    template<typename T>
    T min(T a, T b) { return (((a) < (b)) ? (a) : (b)); }

    template<typename T>
    T abs(T a, T b) { return (((a) < 0) ? -(a) : (a)); }

    template<typename T>
    T clamp(T a, T amin, T amax) { return (min(max((a), (amin)), (amax))); }


    inline double toRadians(double f) { return ((f) / 180.0 * PI); }

    inline double toDegrees(double f) { return  ((f) / PI * 180.0); }

    /**
     * log2(e)
     */
    double const log2e = 1.44269504088896340736; //log2(e)

    /**
     * log base 2.
     */
    inline double log2(double x) {
        return ::log(x) * log2e;
    }

    /**
     * approximately equal.
     * if tolerance is -1, then it is computed using the magnitude.
     */
    inline bool approx(double a, double b, double tolerance = cf_tolerance) {
        double af;
        double bf;
        if (tolerance == -1) {
            af = fabs(a / 1e6);
            bf = fabs(b / 1e6);
            tolerance = min(af, bf);
        }
        return fabs(a - b) < tolerance;
    }

}

/////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include "mgp_pro.h"

typedef std::string Str;

template<typename T> using Array = std::vector<T>;
typedef mgp::Vector3 Vector;
typedef mgp::Rectangle Rect;
typedef mgp::BoundingSphere Sphere;

class Callback {
public:
    virtual ~Callback() {}
    virtual void* call(void*) = 0;
};

#endif // COMMON_H

