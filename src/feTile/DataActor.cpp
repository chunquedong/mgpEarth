#include "feTile/DataActor.h"
#include "mgp.h"
//#include "cppfan/Profiler.h"
#include "feTile/TileData.h"
#include "feTile/TileGeom.hpp"
#include "feElevation/Elevation.h"

PF_USING_NAMESPACE
FE_USING_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ViewState::~ViewState() {
    camera._setRefCount(0);
}


DataActor::DataActor(TileManager* tileManager) : tileManager(tileManager), renderableDirty(false) {
    //tileManager->dataListener = this;
}
DataActor::~DataActor() {
    SAFE_RELEASE(tileManager);
}

void DataActor::sendViewUpdate(Camera* camera, Rectangle* viewport, Matrix* modelMatrix) {
    std::lock_guard<std::mutex> guard(lock);
    if (newestState.modelMatrix == *modelMatrix &&
        newestState.camera.getViewMatrix() == camera->getViewMatrix()) {
        return;
    }
    newestState.camera = *camera;
    newestState.viewport = *viewport;
    newestState.modelMatrix = *modelMatrix;
    
    Message msg;
    //msg.name = "";
    msg.id = 0;
    msg.param = NULL;
    send(msg);

    tileManager->releaseCache();
}

void DataActor::sendMakeData() {
    Message msg;
    //msg.name = "";
    msg.id = 1;
    msg.param = NULL;
    send(msg);
}

void DataActor::onReceive(Message &msg) {
    if (msg.id == 0 || msg.id == 1) {
        if (msg.id == 0) {
            lock.lock();
            curState = newestState;
            lock.unlock();
            tileManager->update(&curState.camera, &curState.viewport, &curState.modelMatrix);
        }
        if (tileManager->resultChanged()) {
            std::vector<TileDataPtr> resultList;
            tileManager->getResult(resultList);

            std::lock_guard<std::mutex> guard(lock);
            renderableList.swap(resultList);
            renderableDirty = true;
        }
    }
}

bool DataActor::mergeMessage(Message *cur, Message *pre) {
    if (cur->id == 0 && cur->id == pre->id) return true;
    return false;
}

void DataActor::getResult(std::vector<TileDataPtr>& list) {
    std::lock_guard<std::mutex> guard(lock);
    renderableList.swap(list);
    renderableDirty = false;
}

bool DataActor::resultChanged() {
    std::lock_guard<std::mutex> guard(lock);
    return renderableDirty;
}

void DataActor::onCancel(Message& msg) {
}
