/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#include "feModel/GeoCoordSys.h"
#include <math.h>
#include "feModel/Mercator.h"

FE_USING_NAMESPACE

const double GeoCoordSys::earthRadius = 6378137.0;

GeoCoordSys* GeoCoordSys::earth() {
  static GeoCoordSys earth(earthRadius);
  return &earth;
}

double GeoCoordSys::distance(const Coord2D p1, const Coord2D p2)
{
  double longitude1 = Math::toRadians(p1.x);
  double latitude1 = Math::toRadians(p1.y);
  double longitude2 = Math::toRadians(p2.x);
  double latitude2 = Math::toRadians(p2.y);

  double d = acos(sin(latitude1)*sin(latitude2)
                  +cos(latitude1)*cos(latitude2)*cos(longitude1-longitude2));
  return radius * d;
}

void GeoCoordSys::blToXyz(const Coord2D p, Vector &out, double radius)
{
  double longitude = Math::toRadians(p.x);
  double latitude = Math::toRadians(p.y);

  double x = radius * cos(longitude) * cos(latitude);
  double y = radius * cos(latitude) * sin(longitude);
  double z = radius * sin(latitude);

  return out.set(x, y, z);
}

void GeoCoordSys::xyzToBl(Vector &vec, Coord2D &p)
{
  vec.normalize();

  double y = asin(vec.z);
  double x = atan2(vec.y, vec.x);

  p.x = Math::toDegrees(x);
  p.y = Math::toDegrees(y);
}

void GeoCoordSys::toMercator(Coord2D *p) {
  double x = p->x * 20037508.34 / 180;
  double y = log(tan((90 + p->y) * Math::PI / 360.0)) / (Math::PI / 180);
  y = y * 20037508.34 / 180;
  p->x = x;
  p->y = y;
}

void GeoCoordSys::fromMercator(Coord2D *p) {
  double x = p->x / 20037508.34 * 180;
  double y = p->y / 20037508.34 * 180;
  y = 180 / Math::PI * (2 * atan(exp(y * Math::PI / 180)) - Math::PI / 2);
  p->x = x;
  p->y = y;
}
