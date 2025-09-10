/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef EARTHCTRL_H
#define EARTHCTRL_H

#include "feTile/TileLayer.h"
#include "feCtrl/EarthAnimation.h"

FE_BEGIN_NAMESPACE

class TrackModel;

class EarthCtrl : public mgp::Refable, public mgp::GestureListener {
protected:
    EarthAnimation* animation;

    //camera to center distance
    double distanceToCenter;
    //long-lat
    Coord2D cameraPosition;
    bool cameraDirty;
    bool _zoomDirty;
    double groundHeight;

    //pitch
    double rotationX;
    //yaw
    double rotationZ;

    //Coord2D lastTouchPosition;
    double fieldOfViewY;

    mgp::Rectangle viewport;

    mgp::Matrix cameraTransform;

    uint64_t lastGroundHeightUpdateTime;

    mgp::UPtr<mgp::Node> _followNode;
    int _followTrackModelId = 0;

    mgp::Picker picker;
public:
    mgp::Node* _groundNode;
    mgp::SceneView* _sceneView;
    double maxLevel;
public:
    
    EarthCtrl();
    ~EarthCtrl();

    void setFollowNode(mgp::Node* node, int followTrackModelId = -1);

    void finalize();

    void invalidateCamera() { cameraDirty = true; }

    EarthAnimation* getAnimation() { return animation; }

    mgp::Picker* getPicker() { return &picker; }
public:

    /**
    * get the scale of world coordinate to screen
    */
    virtual double xyScale();

    /**
    * get current camera look at surface postion in geo coordinate system
    */
    virtual Coord2D getPosition() {
        return cameraPosition;
    }

    double getDistanceToSurface();

  /**
   * set look at surface postion in geo coordinate system
   */
    virtual void moveToPostion(Coord2D pos);

    virtual void setRotationX(double rotX);
    virtual double getRotationX() { return rotationX; }

    virtual void setRotationZ(double rotZ);
    virtual double getRotationZ() { return rotationZ; }

    virtual void setZoom(double dis);
    virtual double getZoom();
    virtual void scaleZoom(double scale);

    /**
    * really do update the camera matrix
    */
    virtual bool updateCamera(float elapsedTime, mgp::Camera &camera, mgp::Rectangle &viewport);

    virtual void moveByPixel(float dx, float dy);

    static bool getGroundPoint(Coord2D position, mgp::Node* node, mgp::Vector3& point);
    static bool getGroundPointByNormal(mgp::Vector3& position, mgp::Node* node, mgp::Vector3& point);
    bool getScreenGroundPoint(Coord2D screen_position, mgp::Vector3& point);
protected:
    virtual void updateCameraTransform(mgp::Camera &camera, mgp::Rectangle &viewport);
    virtual void updateTransform();

private:

    double yDegreeScale(mgp::Rectangle &viewport);
    double xDegreeScale(mgp::Rectangle &viewport, double y);

    void updateGroundHeight(mgp::Node* node);


    void onZoom(float v, int x, int y) override;
    void onPush(float v) override;
    void onMultiTouch(float rotate, float zoom, int x, int y) override;

    void onTouch() override;
    void onDrag(int dx, int dy) override;
    void onFling(float dx, float dy) override;
    void onRightDrag(int dx, int dy) override;
    void onDoubleClick(int dx, int dy) override;
    void onClick(int x, int y) override;
};

FE_END_NAMESPACE
#endif // EARTHCTRL_H
