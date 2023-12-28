#include "GroundNode.h"
#include "feCtrl/EarthCtrl.h"
#include "feModel/GeoCoordSys.h"
#include "feCtrl/EarthApp.h"

FE_USING_NAMESPACE

GroundModel::GroundModel(const char* uri): GltfNode(uri), lastUpdateTime(0), height(0), dirty(true), updateDelay(1000) {
}

void GroundModel::setPosition(const Coord2D& p, double height) {
    if (position.x != p.x || position.y != p.y || this->height != height) {
        dirty = true;
    }
    position = p;
    this->height = height;
}

bool GroundModel::updateHeight(int stickMethod) {
    if (stickMethod == 1) {
        Vector3 target;
        if (app && EarthCtrl::getGroundPoint(position, app->getEarth(), target)) {
            Vector earthPosition;
            GeoCoordSys::blToXyz(position, earthPosition, GeoCoordSys::earth()->getRadius());
            height = earthPosition.distance(target);
            return true;
        }
        else {
            return false;
        }
    }
    else if (stickMethod == 2) {
        if (!OfflineElevation::cur()) return false;
        height = OfflineElevation::cur()->getHeight(position.x, position.y, 18);
        return true;
    }
    return true;
}

void GroundModel::update(float elapsedTime) {
    GeoNode::update(elapsedTime);
    if (!dirty) return;
    if (loadState != 2) return;

    if (autoStickGround) {
        uint64_t now = System::millisTicks();
        if (now - lastUpdateTime > updateDelay) {
            if (updateHeight(autoStickGround)) {
                lastUpdateTime = now;
            }
        }
    }

    Vector3 target;
    GeoCoordSys::blToXyz(position, target, GeoCoordSys::earth()->getRadius()+ height);

    Vector3 dir; target.normalize(&dir);
    Matrix lookAtMatrix;
    Matrix::createLookAt(target + dir * 100, Vector3::zero(), Vector3::unitZ(), &lookAtMatrix, false);

    Matrix rotateMatrix;
    if (!direction.isZero()) {
        Vector3 up = Vector3::unitY();
        Vector3 cross; Vector3::cross(direction, up, &cross);
        Matrix::createRotation(cross, -Vector3::angle(direction, up), &rotateMatrix);
    }
    this->setMatrix(lookAtMatrix * rotateMatrix * pose);

    dirty = false;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrackModel::TrackModel(const char* uri): GltfNode(uri) {
}

void TrackModel::setFromLonLat(std::vector<Coord2D>& path2d, double height) {
    Vector3 target;
    for (auto position : path2d) {
        //double lastGroundHeight = OfflineElevation::cur()->getHeight(position.x, position.y, 18);
        GeoCoordSys::blToXyz(position, target, GeoCoordSys::earth()->getRadius() + height);
        path.push_back(target);
    }
}

void TrackModel::update(float elapsedTime) {
    GltfNode::update(elapsedTime);
    if (!isRuning) return;
    if (loadState != 2) return;

    double advance = elapsedTime / 1000.0 * speed;
    Vector3 target;
    Vector3 direction;
    bool ok = false;
    while (lastPointIndex + 1 < path.size()) {
        Vector3& p0 = path[lastPointIndex];
        Vector3& p1 = path[lastPointIndex + 1];
        double segmentLength = p0.distance(p1);
        
        if (segmentOffset + advance < segmentLength) {
            segmentOffset += advance;
            Vector3 dir = p1 - p0;
            Vector3 p = p0 + dir * (segmentOffset / segmentLength);
            target = p;
            direction = dir.normalize();
            ok = true;
            break;
        }
        else {
            advance -= (segmentLength - segmentOffset);
            segmentOffset = 0;
            lastPointIndex++;
        }
    }

    if (!ok) {
        isRuning = false;
        return;
    }

    Matrix lookAtMatrix;
    Matrix::createLookAt(target, Vector3::zero(), direction, &lookAtMatrix, false);

    this->setMatrix(lookAtMatrix * pose);
}
void TrackModel::start() {
    isRuning = true;
}
void TrackModel::stop() {
    isRuning = false;
}