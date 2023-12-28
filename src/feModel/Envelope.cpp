/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#include "feModel/Envelope.h"
#include <limits.h>
#include <float.h>

FE_USING_NAMESPACE

void Envelope::init(double x1, double y1, double x2, double y2) {
  if (x1 < x2) {
    _minX = x1;
    _maxX = x2;
  } else {
    _minX = x2;
    _maxX = x1;
  }
  if (y1 < y2) {
    _minY = y1;
    _maxY = y2;
  } else {
    _minY = y2;
    _maxY = y1;
  }
}

void Envelope::makeInvalid() {
  _minX = FLT_MAX;
  _maxX = -FLT_MAX;
  _minY = FLT_MAX;
  _maxY = -FLT_MAX;
}

void Envelope::set(double x, double y, double w, double h) {
  double x2 = x + w;
  double y2 = y + h;
  init(x, y, x2, y2);
}

void Envelope::merge(const Envelope &bound) {
  if (_minX > bound._minX) {
    _minX = bound._minX;
  }
  if (_minY > bound._minY) {
    _minY = bound._minY;
  }
  if (_maxX < bound._maxX) {
    _maxX = bound._maxX;
  }
  if (_maxY < bound._maxY) {
    _maxY = bound._maxY;
  }
}

void Envelope::merge(const Coord2D &point) {
  if (_minX > point.x) {
    _minX = point.x;
  }
  if (_minY > point.y) {
    _minY = point.y;
  }
  if (_maxX < point.x) {
    _maxX = point.x;
  }
  if (_maxY < point.y) {
    _maxY = point.y;
  }
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

Envelope Envelope::intersection(const Envelope &env) {
  Envelope r;
  r._minX = MAX(this->_minX, env._minX);
  r._maxX = MIN(this->_maxX, env._maxX);
  r._minY = MAX(this->_minY, env._minY);
  r._maxY = MIN(this->_maxY, env._maxY);
  return r;
}
