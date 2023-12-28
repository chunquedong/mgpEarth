/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#include "feModel/Mercator.h"

FE_USING_NAMESPACE


Spheroid::Spheroid(const char *name, double a, double b) : name(name), a(a), b(b)
{
  assert(!(a < b));
  this->e = computeE();
  this->e2 = computeE2();
}

/**
 */
double Spheroid::computeE()
{
  return sqrt(1.0- pow((b / a), 2));
}

/**
 */
double Spheroid::computeE2()
{
  return sqrt(pow((a / b), 2) - 1.0);
}


const double Mercator::maxDis = 20037508.3427892;
const double Mercator::radius = 6378137.0;

MercatorTransform *Mercator::getTransform()
{
  //Spheroid *s = spheroid();
  //static MercatorTransform trans(s->a, s->b, s->e, s->e2, 0.0, 0.0);
  return &trans;
}

MercatorReTrans *Mercator::getReverseTransform()
{
  //Spheroid *s = spheroid();
  //static MercatorReTrans reTrans(s->a, s->b, s->e, s->e2, 0.0, 0.0);
  return &reTrans;
}

Mercator::Mercator() :
  _spheroid("circle", radius, radius),
  trans(_spheroid.a, _spheroid.b, _spheroid.e, _spheroid.e2, 0.0, 0.0),
  reTrans(_spheroid.a, _spheroid.b, _spheroid.e, _spheroid.e2, 0.0, 0.0)
{
  //name = new Str("Mercator");
}

Mercator &Mercator::getInstance() {
  static Mercator instance;
  return instance;
}

/**
 */
Spheroid *Mercator::spheroid()
{
  //if (cs != NULL && cs->spheroid != NULL) return (Spheroid *)cs->spheroid;
  return &_spheroid;
}

double Mercator::computeK(double a, double b, double e2, double b0)
{
  double fz = pow(a, 2) / b;
  double fm = sqrt(1 + pow(e2, 2) * pow(cos(b0), 2));
  double r = (fz / fm) * cos(b0);
  return r;
}


//=========================================================

MercatorReTrans::MercatorReTrans(double a, double b, double e, double e2, double b0, double l0)
{
  this->k = Mercator::computeK(a, b, e2, b0);
  this->l0 = l0;
  this->e = e;
}

void MercatorReTrans::convert(Coord2D *p)
{
  double B = computeB(p->y);
  double L = getL(p->x);

  double b = Math::toDegrees(B); //longitude
  double l = Math::toDegrees(L); //latitude

  //return new Coord(l, b);
  p->x = l;
  p->y = b;
}

/**
 */
double MercatorReTrans::computeB(double x)
{
  double b = getB(0, x);
  for (int i = 0; i < 20; i++)
  {
    double bb = getB(b, x);
    if (abs(bb - b) < 0.01f) return bb;
    b = bb;
  }
  return b;
}

/**
 */
double MercatorReTrans::getB(double b, double x)
{
  double B = (Math::PI/2) - 2 * atan(my_exp(b,x));
  return B;
}

/**
 */
double MercatorReTrans::my_exp(double b, double x)
{
  return exp(-(x / k))
         * exp(
              (e / 2)* log(
                        (1 - e * sin(b))
                        / (1 + e * sin(b))
                       )
           );
}

/**
 */
double MercatorReTrans::getL(double y)
{
  return (y / k) + l0;
}

//=========================================================

MercatorTransform::MercatorTransform(double a, double b, double e, double e2, double b0, double l0)
{
  this->k = Mercator::computeK(a, b, e2, b0);
  this->l0 = l0;
  this->e = e;
}

void MercatorTransform::convert(Coord2D *p)
{
  double b = Math::toDegrees(p->y); //latitude
  double l = Math::toDegrees(p->x); //longitude

  double y = k * lntan(b, e);
  double x = k * (l - l0);

  //limit
  if (y > Mercator::maxDis)
  {
    y = Mercator::maxDis;
  }
  else if (y < -Mercator::maxDis)
  {
    y = -Mercator::maxDis;
  }

  //return new Coord(x, y);
  p->x = x;
  p->y = y;
}

/**
 */
double MercatorTransform::lntan(double b, double e)
{
  double d = tan(Math::PI / 4 + b / 2) *
       pow(((1 - e * sin(b)) / (1 + e * sin(b))), (e / 2));
  return log(d);
}
