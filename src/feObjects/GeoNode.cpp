#include "GeoNode.h"
#include "feModel/GeoCoordSys.h"
#include "feElevation/Elevation.h"
#include "feCtrl/EarthCtrl.h"

FE_USING_NAMESPACE
PF_USING_NAMESPACE

GeoNode::GeoNode(const char* uri): NetModel(uri) {
}

void GeoNode::update(float elapsedTime) {
    if (!ctrl) {
        return;
    }
    double dis = ctrl->getDistanceToSurface();
    bool visible = false;
    if (dis > minDis && dis <= maxDis) {
        visible = true;
    }
    if (this->isVisiable != visible) {
        this->isVisiable = visible;
        if (visible) {
            if (!loadState) {
                load();
                loadState = 1;
            }
        }
        std::vector<Drawable*> drawables;
        this->getAllDrawable(drawables);
        for (Drawable* d : drawables) {
            d->setVisiable(visible);
        }
    }
    NetModel::update(elapsedTime);
}



GltfNode::GltfNode(const char* uri) : GeoNode(uri) {
}
void GltfNode::scanAttachedFiles(NetResponse &res, std::vector<std::string>& urls, MultiRequest* req) {
    GltfModel::scanBinFile(res.result, urls);
}
void* GltfNode::decodeFile(const char* path, NetResponse& lastRes, MultiRequest* req) {
    return GltfModel::decodeGltf(path, lighting);
}