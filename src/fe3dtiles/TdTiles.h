/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef _TDTILES_H
#define _TDTILES_H

#include "feUtil/common.h"
#include "mgp.h"

#include "jvalue.hpp"

#include <variant>

#include "feTile/TileLayer.h"

FE_BEGIN_NAMESPACE


struct TdTileset;

struct TdCommonBase {
    std::map<std::string, std::string> extensions;
    std::string extras;
};

/** Bounding volume. Blank when not filled yet. Tile with blank bounding volume
 *  cannot be serialized.
 *
 *  boost::blank must stay first template argument otherwise
 *  valid(BoundingVolume) would stop working
 */
using TdBoundingVolume = mgp::BoundingSphere;


struct TdTileContent : TdCommonBase {
    /** Bounding volume can be invalid.
     */
    TdBoundingVolume boundingVolume;
    std::string uri;
};

struct TdTile : TdCommonBase, public ITileData {
    TdBoundingVolume boundingVolume;
    TdBoundingVolume viewerRequestVolume;
    double geometricError = 0.0;

    enum Refinement { REPLACE, ADD };
    Refinement refine;

    TdTileContent content;
    std::vector<TdTile*> children;

    mgp::Matrix transform;

    mgp::UPtr<mgp::Node> renderNode;

    ~TdTile();

    void parse(jc::Value* node);

    virtual mgp::BoundingSphere& bounding();

    virtual int getState();
    virtual mgp::Node* getNode();
    virtual TileKey tileKey();
};

struct TdAsset : TdCommonBase {
    std::string version;
    std::string tilesetVersion;

    TdAsset() : version("1.0") {}
};

struct TdTileset : TdCommonBase {
    TdAsset asset;
    std::map<std::string, std::string> properties;
    double geometricError = 0.0;
    TdTile* root = NULL;

    std::vector<std::string> extensionsUsed;
    std::vector<std::string> extensionsRequired;


    void parse(std::string &str);

    ~TdTileset();
    
};

FE_END_NAMESPACE
#endif
