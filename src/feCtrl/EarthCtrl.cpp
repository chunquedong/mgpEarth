#include "feCtrl/EarthCtrl.h"

//#include "pfRender/GlRenderer.h"
//#include "pfApp/Application.h"
#include "feTile/TileLayer.h"
#include "feModel/GeoCoordSys.h"
//#include "pf2d/DrawNode.h"

FE_USING_NAMESPACE


EarthCtrl::EarthCtrl() : distanceToCenter(0), cameraDirty(true), rotationX(0), rotationZ(0)
  , fieldOfViewY(60), _groundNode(NULL), _sceneView(NULL)
{
    distanceToCenter = GeoCoordSys::earth()->getRadius()*3;
    groundHeight = 0;
    animation = new EarthAnimation();
    animation->init(this);
}
EarthCtrl::~EarthCtrl() {
    animation->release();
}

void EarthCtrl::finalize() {
    picker.finalize();
}

void EarthCtrl::setZoom(double zoom) {
    zoom = Math::clamp(zoom, 0.0, 20.0);
    double dis = (GeoCoordSys::earth()->getRadius()*2 - groundHeight) / pow(2, zoom);
    distanceToCenter =  dis + (GeoCoordSys::earth()->getRadius() + groundHeight);

    invalidateCamera();
}

double EarthCtrl::getZoom() {
    double dis = distanceToCenter - (GeoCoordSys::earth()->getRadius() + groundHeight);
    double p = (GeoCoordSys::earth()->getRadius()*2 - groundHeight) / dis;
    return Math::log2(p);
}

void EarthCtrl::scaleZoom(double scale)
{
    double dis = distanceToCenter - (GeoCoordSys::earth()->getRadius() + groundHeight);
    dis /= scale;
    distanceToCenter =  dis + (GeoCoordSys::earth()->getRadius() + groundHeight);
    invalidateCamera();
}

bool EarthCtrl::updateCamera(float elapsedTime, Camera &camera, Rectangle &viewport)
{
    Node* node = _groundNode;

    animation->update(elapsedTime);
    if (!cameraDirty) {
        return false;
    }
    cameraDirty = false;

    uint64_t now = System::millisTicks();
    if (now - lastGroundHeightUpdateTime > 500) {
        lastGroundHeightUpdateTime = now;
        updateGroundHeight(node);
    }

    if (distanceToCenter < GeoCoordSys::earth()->getRadius()) {
        distanceToCenter = GeoCoordSys::earth()->getRadius();
    }

    updateCameraTransform(camera, viewport);

    //adjust perspective
    double distanceToSurface = getDistanceToSurface();
    double aspect = viewport.width/(double)viewport.height;

    //double farDis = distanceToSurface /cos(Math::toRadians(rotationX)) - distanceToSurface;
    //farDis *= 2;
    //farDis = Math::clamp(farDis, 0.0, 1E6);
    //camera.perspective(fieldOfViewY , aspect
    //                         , distanceToSurface*0.5, distanceToSurface*5);

    camera.setFieldOfView(fieldOfViewY);
    camera.setAspectRatio(aspect);
    camera.setNearPlane(distanceToSurface * 0.01);
    camera.setFarPlane(distanceToSurface * 1000);

    this->viewport = viewport;
    return true;
}

void EarthCtrl::updateCameraTransform(Camera &camera, Rectangle &viewport)
{
    updateTransform();
    camera.getNode()->setMatrix(cameraTransform);
}

bool EarthCtrl::getGroundPoint(Coord2D geo_position, Node* node, Vector3& point) {
    if (!node) return false;
    Vector direction;
    Vector position;
    GeoCoordSys::blToXyz(geo_position, position, GeoCoordSys::earth()->getRadius() * 2);
    position.normalize(&direction);
    direction.negate();
    Ray ray(position, direction);
    std::vector<Drawable*> drawables;
    node->getAllDrawable(drawables);

    RayQuery query;
    query.ray = ray;
    query.pickMask = 2;
    for (Drawable* drawable : drawables) {
        //Vector3 target;
        drawable->raycast(query);
    }
    if (query.minDistance != Ray::INTERSECTS_NONE) {
        point = query.target;
        return true;
    }
    return false;
}

bool EarthCtrl::getScreenGroundPoint(Coord2D screen_position, Vector3& point) {
    if (!_sceneView) return false;
    Camera* _camera = _sceneView->getCamera();
    if (!_groundNode || !_camera) return false;
    Ray ray;
    _camera->pickRay(viewport, screen_position.x, screen_position.y, &ray);
    std::vector<Drawable*> drawables;
    _groundNode->getAllDrawable(drawables);

    RayQuery query;
    query.pickMask = 2;
    query.ray = ray;
    for (Drawable* drawable : drawables) {
        //Vector3 target;
        drawable->raycast(query);
    }
    if (query.minDistance != Ray::INTERSECTS_NONE) {
        point = query.target;
        return true;
    }
    return false;
}

void EarthCtrl::updateGroundHeight(Node* node) {
    Vector3 minTarget;
    if (getGroundPoint(cameraPosition, node, minTarget)) {
        groundHeight = (minTarget.length() - GeoCoordSys::earth()->getRadius());
        if (groundHeight < 0) groundHeight = 0;
    }
}

void EarthCtrl::updateTransform() {
    
    Vector3 originPos(0, 0, 1);
    originPos.scale(distanceToCenter- (GeoCoordSys::earth()->getRadius() + groundHeight));

    Matrix trans;
    Matrix::createTranslation(originPos, &trans);

    Matrix rotateZ;
    Matrix::createRotationZ(Math::toRadians(-rotationZ), &rotateZ);

    Matrix rotateX;
    Matrix::createRotationX(Math::toRadians(rotationX), &rotateX);
    
    Matrix lm =  rotateZ * rotateX * trans;
    
    Vector point;
    GeoCoordSys::blToXyz(cameraPosition, point, GeoCoordSys::earth()->getRadius() + groundHeight);
    Matrix m;
    Matrix::createLookAt(point, Vector(0, 0, 0), Vector(0, 0, 1), &m, false);

    cameraTransform = m * lm;
}

void EarthCtrl::moveToPostion(Coord2D pos) {
    cameraPosition = pos;
    invalidateCamera();
}

void EarthCtrl::setRotationX(double rotX) {
    rotationX = Math::clamp(rotX, 0.0, 85.0);
    invalidateCamera();
}

void EarthCtrl::setRotationZ(double rotZ) {
    rotationZ = rotZ;
    while (rotationZ > 360) {
        rotationZ -= 360;
    }
    while (rotationZ < 0) {
        rotationZ += 360;
    }
    invalidateCamera();
}

void EarthCtrl::moveByPixel(float dx, float dy)
{
    if (dx == 0 && dy == 0) {
        return;
    }
    double x = 0;
    double y = 0;
    if (getZoom() > 7) {
        double scale = 1 / xyScale();

        Vector3 right;
        cameraTransform.getRightVector(&right);
        right.scale(dx * scale);

        Vector3 up;
        cameraTransform.getUpVector(&up);
        up.scale(-dy * scale);

        Vector point;
        GeoCoordSys::blToXyz(cameraPosition, point, GeoCoordSys::earth()->getRadius() + groundHeight);

        Vector3 npoint = point - right - up;
        Coord2D pos;
        GeoCoordSys::xyzToBl(npoint, pos);
        x = pos.x;
        y = pos.y;
    }
    else {
        y = cameraPosition.y + dy / yDegreeScale(viewport);
        x = cameraPosition.x - dx / xDegreeScale(viewport, y);
    }

    if (y < -89) y = -89;
    if (y > 89) y = 89;
    if (x < 180) x += 360;
    if (x > 180) x -= 360;
    
    cameraPosition.x = x;
    cameraPosition.y = y;

    invalidateCamera();
}

double EarthCtrl::getDistanceToSurface() {
    return distanceToCenter - GeoCoordSys::earth()->getRadius() - groundHeight;
}

double EarthCtrl::xyScale()
{
    double distanceToSurface = getDistanceToSurface();
    double width = tan(Math::toRadians(fieldOfViewY/2.0)) * distanceToSurface;
    return (viewport.height/2.0) / width;
}

double EarthCtrl::yDegreeScale(Rectangle &viewport)
{
    double distanceToSurface = getDistanceToSurface();
    double width = tan(Math::toRadians(fieldOfViewY/2.0)) * distanceToSurface;
    double dtan = width / GeoCoordSys::earth()->getRadius();
    double ra = atan(dtan);
    double degree = Math::toDegrees(ra);
    return (viewport.height/2.0) / degree;
}

double EarthCtrl::xDegreeScale(Rectangle &viewport, double y)
{
    double yScale = yDegreeScale(viewport);
    double dcos = cos(Math::toRadians(y));
    return yScale * dcos;
}

void EarthCtrl::onZoom(float v, int x, int y) {
    //setZoom(getZoom()+v*0.5);
    double zoom = getZoom();
    animation->zoomTo(zoom + v * 0.5, 400);
    if (zoom < 8) {
        rotationZ *= 0.8;
        rotationX *= 0.8;
    }
}
void EarthCtrl::onPush(float v) {
    setRotationX(rotationX+v);
}
void EarthCtrl::onMultiTouch(float rotate, float zoom, int x, int y) {
    scaleZoom(zoom);
    setRotationZ(rotationZ + rotate);
}
void EarthCtrl::onTouch() {
    animation->stop();
}
void EarthCtrl::onDrag(int dx, int dy) {
    moveByPixel(dx, dy);
}
void EarthCtrl::onFling(int dx, int dy) {
    animation->fling(dx, dy);
}
void EarthCtrl::onRightDrag(int dx, int dy) {
    setRotationX(rotationX - (dy / 10.0));
    setRotationZ(rotationZ + (dx / 10.0));
}
void EarthCtrl::onDoubleClick(int dx, int dy) {
    /*Vector3 minTarget;
    Coord2D coord(dx, dy);
    if (getScreenGroundPoint(coord, minTarget)) {
        Coord2D pos;
        GeoCoordSys::xyzToBl(minTarget, pos);
        animation->moveTo(pos.x, pos.y, 500);
    }*/
}
void EarthCtrl::onClick(int x, int y) {
    if (!_sceneView) return;
    Camera* _camera = _sceneView->getCamera();
    Node* node = _sceneView->getScene()->getRootNode();
    picker.pickAndHighlight(x, y, _camera, node, *_sceneView->getViewport());
}