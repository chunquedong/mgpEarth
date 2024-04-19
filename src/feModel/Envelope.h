/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "feUtil/common.h"
#include "feModel/Coord2D.h"

FE_BEGIN_NAMESPACE

/**
 * 2D Rectangle range
 */
class Envelope {
public:
  double _minX;
  double _minY;
  double _maxX;
  double _maxY;

public:
  double minX() { return _minX; }
  double minY() { return _minY; }
  double maxX() { return _maxX; }
  double maxY() { return _maxY; }

  /**
   * init by two point
   */
  void init(double x1, double y1, double x2, double y2);

  /**
   * init by xyWH
   */
  void set(double x, double y, double w, double h);

  /**
   * extends to contains bound
   */
  void merge(const Envelope &bound);

  /**
   * extends to contains point
   */
  void merge(const Coord2D &point);

  /**
   * move all coord
   */
  void move(double dx, double dy) {
    _minX += dx;
    _maxX += dx;
    _minY += dy;
    _maxY += dy;
  }

  double width() { return _maxX - _minX; }
  double height() { return _maxY - _minY; }

  Coord2D minPoint() {
    return Coord2D(minX(), minY());
  }

  Coord2D maxPoint() {
    return Coord2D(maxX(), maxY());
  }

  Coord2D leftUp() {
    return Coord2D(minX(), maxY());
  }

  Coord2D rightDown() {
    return Coord2D(maxX(), minY());
  }

  /**
   * expand some buffer
   */
  void expand(double d) {
    _minX -= d;
    _minY -= d;
    _maxX += d;
    _maxY += d;
  }

  /**
   * return true if has intersection
   */
  bool intersects(const Envelope &env) {
    if (this->_maxX < env._minX || env._maxX < this->_minX) {
      return false;
    }
    if (this->_maxY < env._minY || env._maxY < this->_minY) {
      return false;
    }
    return true;
  }

  /**
   * return the intersection
   */
  Envelope intersection(const Envelope &env);

  /**
   * return true if is valid
   */
  bool valid() {
    if (_minX > _maxX) {
      return false;
    }

    if (_minY > _maxY) {
      return false;
    }
    return true;
  }

  /**
   * init a invalid envelope
   */
  void makeInvalid();

  bool equals(const Envelope &env) {
    return (_minX == env._minX) && (_maxX == env._maxX)
    && (_minY == env._minY) && (_maxY == env._maxY);
  }

  bool contains(const Envelope &env) {
    return ((_minX <= env._minX) && (_maxX >= env._maxX)
        && (_minY <= env._minY) && (_maxY >= env._maxY));
  }

  bool containsPoint(double x, double y) {
    return ((_minX <= x) && (_maxX >= x)
        && (_minY <= y) && (_maxY >= y));
  }

  Coord2D getCenter() {
    double x = (_minX + _maxX) / 2.0;
    double y = (_minY + _maxY) / 2.0;
    return Coord2D(x, y);
  }

  double getArea() {
    return height() * width();
  }
};

FE_END_NAMESPACE
#endif // ENVELOPE_H
