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

class EarthApp;

class GroundModel : public GltfNode {
    uint64_t lastUpdateTime;
protected:
    Coord2D position;
    double height;
    mgp::Vector3 direction;
    bool dirty;
public:
    int autoStickGround = 0;
    EarthApp* app = NULL;
    mgp::Matrix pose;
    uint64_t updateDelay;
    GroundModel(const char* uri);
    void setPosition(const Coord2D& p, double height);

    virtual void update(float elapsedTime) override;
    bool updateHeight(int stickMethod = 1);
};

class MultiModel;
class TrackModel : public mgp::Refable {
protected:
    //line segment index
    int lastPointIndex = 0;

    //animation stop time
    uint64_t pathEndTime = 0;

    //animation start time
    uint64_t startTime = 0;

    //offset in curernt line segment
    double segmentOffset = 0;

    //distance from begin to current position in path
    double _offsetLength = 0;
    bool _isRuning = false;
    mgp::Node* _node = nullptr;

    //height update time
    uint64_t lastUpdateTime;
    double _height = 0;
public:
    int _collisionObject = 0;
    MultiModel* parent;
    int autoStickGround = 0;
    uint64_t updateDelay;

    //current position
    mgp::Vector3 _curPosition;

    //current instance id
    int _id;

    //animation speed
    double speed = 15;

    //delay time to call 'onStop' after end animation
    uint64_t afterDelayTime = 0;
    uint64_t beforeDelayTime = 0;

    //init pose matrix
    mgp::Matrix pose;

    //animation along path
    std::vector<mgp::Vector3> path;

    //init model drection
    mgp::Vector3 direction;

    //user define data
    mgp::UPtr<mgp::Refable> userData;

    //callback when animation end
    std::function<void(TrackModel*)> onStop;

    //callback when animation end with delay time
    std::function<void(TrackModel*)> onEnd;

    //callback when position changed
    std::function<void(TrackModel*)> onPositionUpdate;

    TrackModel();
    ~TrackModel();
    
    void setNode(mgp::Node* node);
    mgp::Node* getNode() { return _node; }

    void setFromLonLat(std::vector<Coord2D>& path2d, double height);

    virtual void update(float elapsedTime);
    void start();
    void stop();
    void reset();
    void pause();
    bool isRuning();

    /**
    * play first gltf animation
    */
    void playAnimation(int repeatCount = mgp::AnimationClip::REPEAT_INDEFINITE);
    void stopAnimation();

    /**
    * distance from begin to current position in path
    */
    double getOffsetLength() { return _offsetLength; }
    double getCurSegmentOffset() { return segmentOffset; }
    int getCurSegmentIndex() { return lastPointIndex; }
    double getCurHeight() { return _height; }
private:
    void setStop();
    bool updateHeight(int stickMethod = 1);
protected:
    bool tryUpdateHeight();
};

class MultiModel : public GltfNode, public mgp::PhysicsCollisionObject::CollisionListener {
protected:
    mgp::UPtr<mgp::Node> _templateModel;
    int _idCount = 0;
    std::map<int, mgp::UPtr<TrackModel> > _instances;
public:
    std::function<void(int id1, int id2, const char* layerName1, const char* layer2, const mgp::Vector3& pos)> onCollisionEvent;
    EarthApp* app = NULL;

    MultiModel(const char* uri);

    int add(mgp::UPtr<TrackModel> inst);
    void remove(int id);
    TrackModel* get(int id);
    void clear();

    virtual void update(float elapsedTime) override;
protected:
    virtual void onReceive(mgp::NetResponse& res, mgp::MultiRequest* req) override;

    virtual void collisionEvent(mgp::PhysicsCollisionObject::CollisionListener::EventType type,
        const mgp::PhysicsCollisionObject::CollisionPair& collisionPair,
        const mgp::Vector3& contactPointA = mgp::Vector3::zero(),
        const mgp::Vector3& contactPointB = mgp::Vector3::zero()) override;
};

FE_END_NAMESPACE

#endif // _GEO_LAYER_H
