/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef XYZTILEMANAGER_H
#define XYZTILEMANAGER_H

#include "feModel/Tile.h"
#include "mgp.h"
#include "feTile/TileManager.h"
#include "feElevation/ElevationManager.h"

FE_BEGIN_NAMESPACE

class PyramidGrid;

class XyzTileManager: public TileManager {
    //the root world tile view
    TileDataPtr root;

    std::string cachePath;
    SPtr<ElevationManager> elevation;
    ThreadPool *threadPool;
    SPtr<ElevationQuery> evlevationQuery;
public:
    std::string uri;
    int maxLevel;
    int minLevel;
    PyramidGrid *pyramid;
    float viewScale;

    XyzTileManager(const std::string& uri);
    ~XyzTileManager();

    virtual bool resultChanged();

    void setElevation(ElevationManager* elevation);

    void update(Camera* camera, Rectangle* viewport, Matrix* modelMatrix, bool isSendTask = true) override;

    void setCachePath(const std::string& path);

protected:
    virtual TileDataPtr getRoot();
    virtual TileDataPtr makeTileData(TileKey key);
    virtual void getChildren(TileDataPtr &data, std::vector<TileKey> &children);
    virtual bool isFitLod(TileDataPtr &data, Camera &camera, Rectangle &viewport, Matrix& modelMatrix);
    virtual bool getUri(TileKey key, std::string &uri, std::string &file);
    
    virtual void* decode(Task* task, NetResponse &res);
    virtual void onReceive(Task* task, NetResponse &res);
    virtual void tryInit(TileDataPtr &data, bool isOverview);

    virtual TileDataPtr getParent(TileDataPtr t);
};


FE_END_NAMESPACE

#endif // XYZTILEMANAGER_H
