/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef TILEDATA_H
#define TILEDATA_H

#include "feUtil/common.h"
#include "mgp.h"
#include "feModel/Tile.h"
#include "feModel/Envelope.h"
#include "feTile/TileManager.h"

FE_BEGIN_NAMESPACE
class PyramidGrid;
class TileGeom;

class TileData : public ITileData {
private:
    //geo BL envelope
    CF_FIELD_REF(Envelope, envelope)
    CF_FIELD_REF(Envelope, envelopeMercator)
    
    //tile id
    CF_FIELD_REF(Tile, tile)
    CF_FIELD_REF(Tile, elevationTile)
private:
    //world coord bounds
    Sphere boundingSphere;
    
    PyramidGrid *pyramid;
    mgp::UPtr<mgp::Node> _node;

    bool _isFallback;
public:
    double _approximateHeight;

    virtual mgp::BoundingSphere& bounding();
    virtual int getState();
    virtual mgp::Node* getNode();
    virtual TileKey tileKey();
    virtual void setAsFallback(bool isFallback);

    //attached mesh
    mgp::UPtr<TileGeom> geometry;

    mgp::UPtr<mgp::Image> image;

public:

    TileData(Tile &tile, PyramidGrid *pyramid);
    ~TileData();

    Tile &getTile() { return _tile; }

private:
    //init boundingSphere
    void initBounding();
    void updateTranslation(mgp::Node* node);
public:
    void printTile(const char *name);
public:
    double computeViewScale(mgp::Camera &camera, mgp::Rectangle &viewport);
};


FE_END_NAMESPACE

#endif // TILEDATA_H
