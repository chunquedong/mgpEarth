/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef COORD2D_H
#define COORD2D_H

#include "feUtil/common.h"
#include "mgp.h"

FE_BEGIN_NAMESPACE

/**
 * 2D Coordinate
 */
struct Coord2D {
    double x;
    double y;

    Coord2D() : x(0), y(0) {}
    Coord2D(double x, double y) : x(x), y(y) {
    }

    void set(double x, double y) {
        this->x = x;
        this->y = y;
    }

    double distance(Coord2D &other);
};

FE_END_NAMESPACE
#endif // COORD2D_H
