/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef EARTHANIMATION_H
#define EARTHANIMATION_H

#include "feUtil/common.h"
#include "mgp.h"
#include "feModel/Coord2D.h"

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE

class EarthCtrl;

class AnimChannel {
public:
    uint64_t duration = 1000;
    bool isRunning = false;
    float _elapsedTime = 0;

    bool update(float elapsedTime);
    void stop();
    void start();
protected:
    virtual void doUpdate(float elapsedTime, float percentComplete) = 0;
};

class FlingAnimChannel : public AnimChannel
{
public:
  EarthCtrl *ctrl = NULL;
  float dx;
  float dy;
  float acceleratedX;
  float acceleratedY;

  virtual void doUpdate(float elapsedTime, float percentComplete);
};

class ZoomAnimChannel : public AnimChannel {
public:
    EarthCtrl* ctrl = NULL;
    float from = 0;
    float to = 0;

    virtual void doUpdate(float elapsedTime, float percentComplete);
};

class RotateAnimChannel : public AnimChannel {
public:
    EarthCtrl* ctrl = NULL;
    float fromRotateX = 0;
    float fromRotateZ = 0;
    float toRotateX = 0;
    float toRotateZ = 0;

    virtual void doUpdate(float elapsedTime, float percentComplete);
};

class MoveToAnimChannel : public AnimChannel {
public:
    EarthCtrl* ctrl = NULL;
    double fromX = 0;
    double fromY = 0;
    double toX = 0;
    double toY = 0;
    double fromZoom = 0;
    double toZoom = 0;
    double middleZoom = 0;

    virtual void doUpdate(float elapsedTime, float percentComplete);
};

class EarthAnimation : public Refable {
    FlingAnimChannel *flingChannel = NULL;
    ZoomAnimChannel* zoomChannel = NULL;
    RotateAnimChannel* rotateChannel = NULL;
    MoveToAnimChannel* moveChannel = NULL;
    EarthCtrl *ctrl = NULL;
    std::vector<AnimChannel*> channelList;
public:

    EarthAnimation();
    ~EarthAnimation();

    void start(AnimChannel* chnnel);
    void stop();

    bool update(float elapsedTime);

    void init(EarthCtrl *ctrl);

    void fling(float dx, float dy);
    void zoomTo(float zoom, uint64_t time);
    void rotateTo(float rx, float rz, uint64_t time);
    void moveTo(double x, double y, uint64_t time, double zoom = NAN);
};

FE_END_NAMESPACE
#endif // EARTHANIMATION_H
