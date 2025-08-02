/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef _T3DLayer_H
#define _T3DLayer_H

#include "feUtil/common.h"
#include "mgp.h"
#include "TdTiles.h"
#include "feTile/TileLayer.h"

FE_BEGIN_NAMESPACE

struct TdTilesData;
struct TdTile;

using namespace mgp;


class TdTilesManager : public TileManager {

    TdTileset tileset;

    std::string uri;

    std::map<std::string, std::string> tokens;

public:
    int lighting = 0;

    TdTilesManager(const std::string& uri);
    ~TdTilesManager();
    //unsigned int draw(RenderInfo* view) override;
    //virtual void update(float elapsedTime) override;
    //virtual const BoundingSphere& getBoundingSphere() const override;

    virtual void* decode(Task* task, NetResponse &res) override;
    virtual void onReceive(Task* task, NetResponse &res) override;

    void init();

    virtual mgp::Matrix* getRootTransform() override;
private:
    void loadData(Tile* tile);

    virtual TileDataPtr getRoot();
    virtual TileDataPtr makeTileData(TileKey key);
    virtual void getChildren(TileDataPtr &data, std::vector<TileKey> &children);
    virtual bool isFitLod(TileDataPtr &data, Camera &camera, Rectangle &viewport, Matrix& modelMatrix);
    virtual bool getUri(TileKey key, std::string &uri, std::string &file);
};

FE_END_NAMESPACE

#endif
