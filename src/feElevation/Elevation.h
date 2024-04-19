/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef ELEVATION_H
#define ELEVATION_H

#include "feModel/Tile.h"
#include "mgp.h"
#include "feTile/TileManager.h"
#include "feTile/TileData.h"

FE_BEGIN_NAMESPACE


class Elevation {
public:
    virtual double getHeight(double longitude, double latitude, int level) = 0;
    virtual double getHeightMercator(double x, double y, int level) = 0;
};

class OfflineElevation: public Elevation {
protected:
    void* imageData;
    int width;
    int height;
    Envelope envelope;
    int format;
public:
    double elevationScale;
protected:
    OfflineElevation();
public:
    static OfflineElevation* cur();
    static void _setCur(OfflineElevation* elev);
    OfflineElevation(const char* uri, int format = 2, Envelope* env = NULL);
    virtual double getHeight(double longitude, double latitude, int level) override;
    virtual double getHeightMercator(double x, double y, int level) override;
private:
    double sampleHeight(int x, int y);
};


FE_END_NAMESPACE

#endif // ELEVATION_H