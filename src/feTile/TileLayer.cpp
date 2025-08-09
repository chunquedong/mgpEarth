#include "feTile/TileLayer.h"
#include "mgp.h"
//#include "cppfan/Profiler.h"
#include "feTile/TileData.h"


TileLayer::TileLayer(TileManager* tileManager) {
    //delayUpdater.setMaxUpdateDelay(20);
    this->tileManager = tileManager;
    //this->tileManager->start();
}
TileLayer::~TileLayer() {
    //if (tileManager) tileManager->stop();
    SAFE_RELEASE(tileManager);
}

void TileLayer::update(float elapsedTime) {
    Camera* camera = getScene()->getActiveCamera();
    if (!camera) return;
    //TODO
    Rectangle viewport(0, 0, 1920, 1080);

    if (!isTransformInited) {
        Matrix* rootTransform = tileManager->getRootTransform();
        if (rootTransform) {
            isTransformInited = true;
            Matrix matrix = (*rootTransform) * this->getMatrix();
            this->setMatrix(matrix);
        }
    }

    Matrix matrix = getWorldMatrix();
    tileManager->update(camera, &viewport, &matrix);

    if (tileManager->resultChanged()) {
        std::vector<TileDataPtr> resultList;
        tileManager->getResult(resultList);

        removeAllChildren();
        for (TileDataPtr& tile : resultList) {
            addChild(uniqueFromInstant(tile->getNode()));
        }
    }
    
    Node::update(elapsedTime);
}

float TileLayer::getProgress() {
    return tileManager->getProgress();
}
