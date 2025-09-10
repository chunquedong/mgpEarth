/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef GEOCOORDSYS_H
#define GEOCOORDSYS_H

#include "feUtil/common.h"
#include "mgp.h"
#include "feModel/Coord2D.h"

FE_BEGIN_NAMESPACE

class GeoCoordSys {
  double radius;
  static const double earthRadius;
public:
  GeoCoordSys(double radius) : radius(radius) {
  }

  double getRadius() { return radius; }

  static GeoCoordSys* earth();

  /**
  ** Get the spherical distance of two geographic coordinates points
  **/
  double distance(const Coord2D p1, const Coord2D p2);

  /**
  ** From spherical coordinates to rectangular space coordinates
  **/
  void lnglatToXyz(const Coord2D& lnglat, double height, Vector &out) {
	  blToXyz(lnglat, out, radius + height);
  }

  void xyzToLnglat(const Vector& point, Coord2D& lnglat, double* height) {
	  xyzToBl(point, lnglat);
	  if (height)
		  *height = point.length() - radius;
  }

  static void blToXyz(const Coord2D p, Vector &out, double radius);

  static void xyzToBl(const Vector &vec, Coord2D &p);

  void toMercator(Coord2D *p);

  void fromMercator(Coord2D *p);
};

FE_END_NAMESPACE

#endif // GEOCOORDSYS_H
