/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef MERCATOR_H
#define MERCATOR_H

#include "feUtil/common.h"
#include "feModel/Coord2D.h"

FE_BEGIN_NAMESPACE


/**
 * Earth ellipsoid
 *
 */
class Spheroid {
  /**
   *
   */
  public: const char *name;

  /**
   *
   */
  public: const double a;

  /**
   *
   */
  public: const double b;

  /**
   *
   */
  public: double e;

  /**
   *
   */
  public: double e2;


  //=========================================================

  /**
   *
   */
  public: Spheroid(const char *name, double a, double b);

  private: double computeE();

  private: double computeE2();

};

/**
  */
 class MercatorReTrans {
   /**
    */
   private: double k;

   /**
    */
   private: double l0;

   /**
    */
   private: double e;

   /**
    */
   public: virtual void* call(void* a) { convert((Coord2D *)a); return (Coord2D *)a; }


   //=========================================================

   /**
    */
   public: MercatorReTrans(double a, double b, double e, double e2, double b0, double l0);

   /**
    */
   public: void convert(Coord2D *p);

   private: double computeB(double x);

   private: double getB(double b, double x);

   private: double my_exp(double b, double x);

   private: double getL(double y);

 };

 /**
  */
 class MercatorTransform
 {
   /**
    */
   private: double k;

   /**
    */
   private: double l0;

   /**
    */
   private: double e;

   /**
    */
   public: virtual void* call(void* a) { convert((Coord2D *)a); return (Coord2D *)a; }


   //=========================================================

   /**
    */
   public: MercatorTransform(double a, double b, double e, double e2, double b0, double l0);

   /**
    */
   public: void convert(Coord2D *p);

   private: double lntan(double b, double e);

 };

/**
 * Mercator Projection
 *
 */
class Mercator {
private:
  Spheroid _spheroid;
  MercatorTransform trans;
  MercatorReTrans reTrans;

  /**
   * maximum latitude equals to half of width
   *
   */
  public: static const double maxDis;

  /**
   * radius of earth
   *
   */
  public: static const double radius;


  //=========================================================

  /**
   *
   */
  public: virtual MercatorTransform *getTransform();

  /**
   *
   */
  public: virtual MercatorReTrans *getReverseTransform();

  /**
   *
   */
  public: Mercator();

  private: Spheroid *spheroid();

public:
  static Mercator &getInstance();

  /**
   *
   */
  public: static double computeK(double a, double b, double e2, double b0);

};


FE_END_NAMESPACE
#endif // MERCATOR_H
