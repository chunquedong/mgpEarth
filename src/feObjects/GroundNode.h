/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef _GROUNDMODEL_H
#define _GROUNDMODEL_H

#include "feUtil/common.h"
#include "mgp_pro.h"
#include "feModel/Envelope.h"
#include "GeoNode.h"

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE
class EarthApp;
class GroundModel : public GltfNode {
    uint64_t lastUpdateTime;
protected:
    Coord2D position;
    double height;
    Vector3 direction;
    bool dirty;
public:
    int autoStickGround = 0;
    EarthApp* app = NULL;
    Matrix pose;
    uint64_t updateDelay;
    GroundModel(const char* uri);
    void setPosition(const Coord2D& p, double height);

    virtual void update(float elapsedTime) override;
    bool updateHeight(int stickMethod = 1);
};

class TrackModel : public GltfNode {
    int lastPointIndex = 0;
    uint64_t lastTime = 0;
    double segmentOffset = 0;
    bool isRuning = false;
public:
    double speed = 15;
    Matrix pose;
    std::vector<Vector3> path;
    void setFromLonLat(std::vector<Coord2D>& path2d, double height);
    TrackModel(const char* uri);
    virtual void update(float elapsedTime);
    void start();
    void stop();
};

FE_END_NAMESPACE

#endif // _GEO_LAYER_H
