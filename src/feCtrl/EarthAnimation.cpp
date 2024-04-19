#include "feCtrl/EarthAnimation.h"
#include "feCtrl/EarthCtrl.h"
#include "feModel/GeoCoordSys.h"

FE_USING_NAMESPACE

EarthAnimation::EarthAnimation()
{
}
EarthAnimation::~EarthAnimation() {
    delete flingChannel;
    delete zoomChannel;
    delete rotateChannel;
    delete moveChannel;
    channelList.clear();
}

void EarthAnimation::init(EarthCtrl *ctrl)
{
    this->ctrl = ctrl;
    flingChannel = new FlingAnimChannel();
    flingChannel->ctrl = ctrl;

    zoomChannel = new ZoomAnimChannel();
    zoomChannel->ctrl = ctrl;

    rotateChannel = new RotateAnimChannel();
    rotateChannel->ctrl = ctrl;

    moveChannel = new MoveToAnimChannel();
    moveChannel->ctrl = ctrl;
}

bool EarthAnimation::update(float elapsedTime) {
    for (auto it = channelList.begin(); it != channelList.end();) {
        AnimChannel* c = *it;
        c->update(elapsedTime);
        if (!c->isRunning) {
            it = channelList.erase(it);
        }
        else {
            ++it;
        }
    }
    return true;
}

void EarthAnimation::start(AnimChannel* chnnel) {
    chnnel->start();
    this->channelList.push_back(chnnel);
}

void EarthAnimation::stop() {
    for (auto it = channelList.begin(); it != channelList.end(); ++it) {
        (*it)->stop();
    }
    channelList.clear();
}

void AnimChannel::stop() {
    isRunning = false;
}
void AnimChannel::start() {
    _elapsedTime = 0;
    isRunning = true;
}

bool AnimChannel::update(float elapsedTime) {
    if (!isRunning) return false;
    _elapsedTime += elapsedTime;
    float percentComplete = 0;
    percentComplete = _elapsedTime / (float)duration;
    if (percentComplete > 1) {
        isRunning = false;
        percentComplete = 1;
    }

    doUpdate(elapsedTime, percentComplete);
    return true;
}

void EarthAnimation::fling(float dx, float dy)
{
    if (fabs(dx) <= 1 && fabs(dy) <=1) {
        return;
    }

    flingChannel->dx = dx;
    flingChannel->dy = dy;
    //printf("animation start: %f, %f\n", dx, dy);

    float acc = 0.3f;
    if (dx > 0) {
        flingChannel->acceleratedX = -acc;
    }
    else {
        flingChannel->acceleratedX = acc;
    }
    if (dy > 0) {
        flingChannel->acceleratedY = -acc;
    }
    else {
        flingChannel->acceleratedY = acc;
    }

    start(flingChannel);
}

void EarthAnimation::zoomTo(float zoom, uint64_t time) {
    zoomChannel->from = ctrl->getZoom();
    zoomChannel->to = zoom;
    zoomChannel->duration = time;
    start(zoomChannel);
}

void EarthAnimation::rotateTo(float rx, float rz, uint64_t time) {
    rotateChannel->fromRotateX = ctrl->getRotationX();
    rotateChannel->toRotateX = rx;
    rotateChannel->fromRotateZ = ctrl->getRotationZ();
    rotateChannel->toRotateZ = rz;
    rotateChannel->duration = time;
    start(rotateChannel);
}

void EarthAnimation::moveTo(double x, double y, uint64_t time, double zoom) {
    Coord2D pos = ctrl->getPosition();
    moveChannel->fromX = pos.x;
    moveChannel->toX = x;
    moveChannel->fromY = pos.y;
    moveChannel->toY = y;
    moveChannel->fromZoom = ctrl->getZoom();

    if (isnan(zoom)) {
        moveChannel->toZoom = moveChannel->fromZoom;
    }
    else {
        moveChannel->toZoom = zoom;
    }

    Coord2D newPos(x, y);
    double dis = GeoCoordSys::earth()->distance(pos, newPos);
    double fids = GeoCoordSys::earth()->getRadius() / (1 << (int)(moveChannel->fromZoom));
    if (dis > fids * 4) {
        moveChannel->middleZoom = std::min(moveChannel->fromZoom, moveChannel->toZoom) - 4;
    }
    else {
        moveChannel->middleZoom = (moveChannel->fromZoom + moveChannel->toZoom) / 2;
    }

    moveChannel->duration = time;
    start(moveChannel);
}

void FlingAnimChannel::doUpdate(float elapsedTime, float percentComplete)
{
    if (acceleratedX == 0 && acceleratedY == 0) return;

    float nx = dx + acceleratedX * elapsedTime;
    float ny = dy + acceleratedY * elapsedTime;

    //printf("animation: %f, %f => %f, %f\n", dx, dy, nx, ny);

    if (nx * dx <= 0) {
        dx = 0;
        acceleratedX = 0;
    }
    else {
        dx = nx;
    }
    if (ny * dy <= 0) {
        dy = 0;
        acceleratedY = 0;
    }
    else {
        dy = ny;
    }

    if (dx != 0 || dy != 0) {
        ctrl->moveByPixel(dx, dy);
    }
}

void ZoomAnimChannel::doUpdate(float elapsedTime, float percentComplete) {
    float d = to - from;
    float zoom = d * percentComplete + from;
    ctrl->setZoom(zoom);
}

void RotateAnimChannel::doUpdate(float elapsedTime, float percentComplete) {
    float dx = toRotateX - fromRotateX;
    float rx = dx * percentComplete + fromRotateX;
    ctrl->setRotationX(rx);

    float dz = toRotateZ - fromRotateZ;
    float rz = dz * percentComplete + fromRotateZ;
    ctrl->setRotationZ(rz);
}

void MoveToAnimChannel::doUpdate(float elapsedTime, float percentComplete) {
    float dx = toX - fromX;
    float rx = dx * percentComplete + fromX;
    float dz = toY - fromY;
    float rz = dz * percentComplete + fromY;

    ctrl->moveToPostion(Coord2D(rx, rz));

    float from = fromZoom;
    float to = toZoom;
    if (percentComplete < 0.5) {
        to = middleZoom;
    }
    else {
        from = middleZoom;
    }
    if (to != from) {
        float d = to - from;
        float zoom = d * percentComplete + from;
        ctrl->setZoom(zoom);
    }
}
