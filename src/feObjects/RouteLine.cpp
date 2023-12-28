#include "RouteLine.h"
#include "feCtrl/EarthCtrl.h"
#include "feModel/GeoCoordSys.h"


FE_USING_NAMESPACE

RouteLine::RouteLine(): GeoNode("") {
    style.lineColor = Vector4(0.0, 0.0, 1.0, 1.0);
    style.lineWidth = 8;
}

void RouteLine::update(float elapsedTime) {
    GeoNode::update(elapsedTime);
    /*
    if (!ctrl) {
        return;
    }
    int level = (int)ctrl->getZoom();
    if (level == updateLevel) {
        return;
    }
    updateLevel = level;*/
    if (line) {
        return;
    }

    std::vector<float> data;
    data.reserve(path.size()*3);
    Vector3 translation;
    Vector3 target;
    for (int i = 0; i < path.size(); ++i) {
        Coord2D& position = path[i];
        //double lastGroundHeight = OfflineElevation::cur()->getHeight(position.x, position.y, 18);
        GeoCoordSys::blToXyz(position, target, GeoCoordSys::earth()->getRadius() + height);
        if (i == 0) {
            translation = target;
        }
        target -= translation;
        data.push_back(target.x);
        data.push_back(target.y);
        data.push_back(target.z);
    }

    if (!line) {
        auto linePtr = MeshLine::create(style);
        line = linePtr.get();
        this->setDrawable(linePtr.dynamicCastTo<Drawable>());
    }
    this->setTranslation(translation);

    line->start();
    line->normal = translation;
    line->normal.normalize();
    line->_style.lineWidth = style.lineWidth;
    line->add(data.data(), path.size());
    line->finish();
}