#include "feTile/TileData.h"
#include "mgp.h"
#include "feModel/PyramidGrid.h"
#include "feModel/GeoCoordSys.h"
#include "feTile/TileGeom.hpp"

FE_USING_NAMESPACE
PF_USING_NAMESPACE

TileData::TileData(Tile &tile, PyramidGrid *pyramid) : _tile(tile),
   geometry(nullptr), pyramid(pyramid), _node(nullptr), image(nullptr), _isFallback(false), _approximateHeight(0)
{
    pyramid->tileEnvelopeBL(_tile, _envelope);
    pyramid->tileEnvelope(_tile, _envelopeMercator);
}

TileData::~TileData() {
    /*SAFE_RELEASE(geometry);
    SAFE_RELEASE(_node);
    SAFE_RELEASE(image);*/
}

mgp::BoundingSphere& TileData::bounding() {
    if (geometry.get() && geometry->getNode()) {
        boundingSphere = (mgp::BoundingSphere&)geometry->getMesh()->getBoundingSphere();
        boundingSphere.center += geometry->getTranlation();
        return boundingSphere;
    }
    initBounding();
    return boundingSphere;
}

int TileData::getState() {
    if (geometry.get()) return 2;
    if (image.get()) return 1;
    return 0;
}

TileKey TileData::tileKey() {
    TileKey key = {};
    key.tile = _tile;
    return key;
}

void TileData::updateTranslation(Node* node) {
    node->setTranslation(geometry->getTranlation());
    if (_isFallback) {
        Vector3 v = boundingSphere.center;
        v.normalize();
        v.negate();
        int num = 1 << tile().z;
        double adjust = 65535.0 / num;
        if (adjust < 0) adjust = 0;
        node->translate(v * adjust);
    }
}

Node* TileData::getNode() {
    if (_node.get()) {
        updateTranslation(_node.get());
        return _node.get();
    }
    if (!geometry.get()) return nullptr;

    char buf[256];
    snprintf(buf, 256, "%d_%d_%d", _tile.x, _tile.y, _tile.z);
    _node = Node::create(buf);
    _node->addComponent(uniqueFromInstant(geometry.get()));
    //_node->setTranslation(geometry->getTranlation());
    updateTranslation(_node.get());
    return _node.get();
}

void TileData::setAsFallback(bool isFallback) {
    _isFallback = isFallback;
}

double TileData::computeViewScale(Camera &camera, Rectangle &viewport)
{
    Vector center;
    Coord2D c = envelope().getCenter();
    GeoCoordSys::earth()->lnglatToXyz(c, _approximateHeight, center);
    camera.getViewMatrix().transformPoint(&center);

    Vector position;
    position.set(0, 0, 0);
    
    double distance = center.distance(position);
    double surfaceHeight = tan(Math::toRadians(camera.getFieldOfView()/2.0)) * distance;
    double geoHeight = Math::toRadians(envelope().height()) * (GeoCoordSys::earth()->getRadius()+ _approximateHeight);

    double height = viewport.height * geoHeight / (surfaceHeight+surfaceHeight);

    int w,h;
    pyramid->tileScreenSize(w, h);

    float dpiScale = Toolkit::cur()->getScreenScale();//Device::cur()->dpToPixelScale();
    if (dpiScale < 2) {
        dpiScale = 1;
    }

    //display size / image size
    double viewScale = (height * height ) / (w*h *(dpiScale*dpiScale));
    return viewScale * 0.5;
}

static void project(Coord2D coord, Sphere &boundingSphere, double height) {
    Vector point;
    GeoCoordSys::earth()->blToXyz(coord, point, GeoCoordSys::earth()->getRadius() + height);
    boundingSphere.merge(point);
}

void TileData::initBounding() {
    if (!boundingSphere.isEmpty()) {
      return;
    }
    Envelope &envelope = _envelope;
    Tile &tile = _tile;
    
    project(envelope.minPoint(), boundingSphere, _approximateHeight);
    project(envelope.maxPoint(), boundingSphere, _approximateHeight);
    project(envelope.leftUp(), boundingSphere, _approximateHeight);
    project(envelope.rightDown(), boundingSphere, _approximateHeight);
    project(envelope.getCenter(), boundingSphere, _approximateHeight);
    
    if (tile.z < 16) {
        Coord2D coord;
        coord.set(envelope.minX()+envelope.width()/4, envelope.minY()+envelope.height()/4);
        project(coord, boundingSphere, _approximateHeight);
        
        coord.set(envelope.minX()+envelope.width()/4*3, envelope.minY()+envelope.height()/4);
        project(coord, boundingSphere, _approximateHeight);
        
        coord.set(envelope.minX()+envelope.width()/4, envelope.minY()+envelope.height()/4*3);
        project(coord, boundingSphere, _approximateHeight);
        
        coord.set(envelope.minX()+envelope.width()/4*3, envelope.minY()+envelope.height()/4*3);
        project(coord, boundingSphere, _approximateHeight);
    }

    //height is approximate
    if (tile.z > 8) {
        boundingSphere.radius *= 2;
    }
    if (tile.z > 13) {
        boundingSphere.radius *= 2;
    }
    if (tile.z > 15) {
        boundingSphere.radius *= 2;
    }
}

void TileData::printTile(const char *name)
{
    printf("%s: %d,%d,%d. ref:%d, inMem:%d, fallback:nil\n", name, tile().x, tile().y, tile().z
             , getRefCount(), isReady());
    fflush(stdout);
}
