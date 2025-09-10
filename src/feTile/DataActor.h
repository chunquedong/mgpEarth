/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef DATAACTOR_H
#define DATAACTOR_H

#include "mgp_pro.h"
#include "feTile/XyzTileManager.h"

FE_BEGIN_NAMESPACE

struct ViewState {
    mgp::Camera camera;
    mgp::Rectangle viewport;
    mgp::Matrix modelMatrix;
    ~ViewState();
};

class DataActor : public mgp::SimpleActor {
    ViewState newestState;
    ViewState curState;

    std::vector<TileDataPtr> renderableList;
    bool renderableDirty;

    TileManager* tileManager;

    std::mutex lock;
public:
    DataActor(TileManager* tileManager);
    ~DataActor();

    void sendMakeData();
    void sendViewUpdate(mgp::Camera* camera, mgp::Rectangle* viewport, mgp::Matrix* modelMatrix);

    void getResult(std::vector<TileDataPtr>& list);
    bool resultChanged();

    virtual void onReceive(Message &msg) override;
    virtual void onCancel(Message& msg) override;
    virtual bool mergeMessage(Message *cur, Message *pre);
};


FE_END_NAMESPACE

#endif // DATAACTOR_H
