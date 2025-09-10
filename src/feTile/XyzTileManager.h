/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef XYZTILEMANAGER_H
#define XYZTILEMANAGER_H

#include "feModel/Tile.h"
#include "mgp.h"
#include "feTile/TileManager.h"
#include "feElevation/ElevationManager.h"

#ifndef __EMSCRIPTEN__
    #define MGP_THREAD 1
#endif

FE_BEGIN_NAMESPACE

class PyramidGrid;

class XyzTileManager: public TileManager {
    //the root world tile view
    TileDataPtr root;

    std::string cachePath;
    mgp::SPtr<ElevationManager> elevation;
#ifdef MGP_THREAD
    mgp::ThreadPool *threadPool;
#endif
    mgp::SPtr<ElevationQuery> evlevationQuery;
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

    void update(mgp::Camera* camera, mgp::Rectangle* viewport, mgp::Matrix* modelMatrix, bool isSendTask = true) override;

    void setCachePath(const std::string& path);

protected:
    virtual TileDataPtr getRoot();
    virtual TileDataPtr makeTileData(TileKey key);
    virtual void getChildren(TileDataPtr &data, std::vector<TileKey> &children);
    virtual bool isFitLod(TileDataPtr &data, mgp::Camera &camera, mgp::Rectangle &viewport, mgp::Matrix& modelMatrix);
    virtual bool getUri(TileKey key, std::string &uri, std::string &file);
    
    virtual void* decode(mgp::Task* task, mgp::NetResponse &res);
    virtual void onReceive(mgp::Task* task, mgp::NetResponse &res);
    virtual void tryInit(TileDataPtr &data, bool isOverview);

    virtual TileDataPtr getParent(TileDataPtr t);
};


FE_END_NAMESPACE

#endif // XYZTILEMANAGER_H
