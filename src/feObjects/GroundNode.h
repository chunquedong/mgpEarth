/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
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

class TrackModel : public Refable {
    int lastPointIndex = 0;
    uint64_t pathEndTime = 0;
    double segmentOffset = 0;
    bool isRuning = false;
    Node* _node = nullptr;
public:
    int _id;
    double speed = 15;
    uint64_t afterDelayTime = 0;
    Matrix pose;
    std::vector<Vector3> path;
    Vector3 direction;
    UPtr<Refable> userData;

    std::function<void(TrackModel*)> onStop;

    TrackModel();
    ~TrackModel();
    
    void setNode(Node* node);
    Node* getNode() { return _node; }

    void setFromLonLat(std::vector<Coord2D>& path2d, double height);
    
    virtual void update(float elapsedTime);
    void start();
    void stop();
    void reset();
    void pause();
    void playAnimation(int repeatCount = AnimationClip::REPEAT_INDEFINITE);
private:
    void setStop();
};

class MultiModel : public GltfNode {
protected:
    UPtr<Node> _templateModel;
    int _idCount = 0;
    std::map<int, UPtr<TrackModel> > _instances;
public:
    MultiModel(const char* uri);

    int add(UPtr<TrackModel> inst);
    void remove(int id);
    TrackModel* get(int id);
    void clear();

    virtual void update(float elapsedTime) override;
protected:
    virtual void onReceive(Task* task, NetResponse& res, MultiRequest* req) override;
};

FE_END_NAMESPACE

#endif // _GEO_LAYER_H
