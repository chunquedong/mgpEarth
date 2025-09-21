/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef ELEVATION_MANAGER_H
#define ELEVATION_MANAGER_H

#include "Elevation.h"

FE_BEGIN_NAMESPACE

class ElevationManager;
class ElevationQuery : public mgp::Refable, public Elevation {
    friend class ElevationManager;
    //TileData* _lastTile = nullptr;
    mgp::WeakPtr<ElevationManager> manager;
public:
    bool resultDirty = false;
    ElevationQuery();
    ~ElevationQuery();
    mgp::HashMap<Tile, mgp::SPtr<TileData> > tiles;
    bool isLoaded();

    double getHeight(double longitude, double latitude, int level);
    double getHeightMercator(double x, double y, int level);
    double getTileHeight(Tile tile, double x, double y, double defVal, int level);

    //virtual void onReceive(SPtr<TileData> tileData);

    double getHeightUV(TileData* tileData, double u, double v, double defVal, int level);
    double getTileHeight(TileData* tileData, double x, double y, double defVal, int level);
};

class ElevationRequest : public mgp::HttpClient {
public:
    int queryCount = 0;
    Tile tileKey{};
};

class TileData;
class ElevationManager : public mgp::Refable, public mgp::NetListener {
protected:
    std::string uri;

    LRUCache cache;
    std::list<ElevationQuery*> querys;
    mgp::HashMap<Tile, sric::SharedPtr<ElevationRequest> > sendedTask;

    PyramidGrid* pyramid;
    std::string cachePath;

    //ElevationQuery approximateElevation;
public:
    int maxLevel;
    int minLevel;
    double elevationScale;
    ElevationManager(const std::string& uri);
    ~ElevationManager();

    //ElevationQuery* getApproximateElevation() { return &approximateElevation; }

    mgp::SPtr<ElevationQuery> fromTiles(std::set<Tile>& tiles);
    Tile getTileAt(double x, double y, int level);
    Tile getTileAtBL(double longitude, double latitude, int level);
    void cancel(ElevationQuery* query);

    virtual void* decode(mgp::HttpClient* task, mgp::NetResponse& res);
    virtual void onReceive(mgp::HttpClient* task, mgp::NetResponse& res);

    virtual bool getUri(TileKey key, std::string& uri, std::string& file);
    void setCachePath(const std::string& path);
private:
    sric::SharedPtr<ElevationRequest> load(Tile key);
    
};


FE_END_NAMESPACE

#endif // ELEVATION_MANAGER_H