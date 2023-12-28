/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef TILELAYER_H
#define TILELAYER_H

#include "mgp.h"
#include "feTile/XyzTileManager.h"


FE_BEGIN_NAMESPACE

class TileLayer : public Node {
    TileManager* tileManager;
    //DelayUpdater delayUpdater;
public:

    //void setViewDirty() { delayUpdater.setDirty(); }
    //unsigned int draw(RenderInfo* view) override;
    virtual void update(float elapsedTime) override;
public:
    TileLayer(TileManager* tileManager);
    ~TileLayer();
};

FE_END_NAMESPACE

#endif // TILELAYER_H
