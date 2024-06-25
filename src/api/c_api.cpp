/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#include "mgpEarth.h"
#include "mgp_pro.h"
#include "jparser.hpp"
#include <stdio.h>

#ifndef __EMSCRIPTEN__
    #define EMSCRIPTEN_KEEPALIVE 
#else
    #include <emscripten.h>
#endif

using namespace jc;

extern "C" {


void EMSCRIPTEN_KEEPALIVE fe_showFps(EarthApp* self, bool show) {
    self->showFps(show);
}

bool EMSCRIPTEN_KEEPALIVE fe_removeNode(EarthApp* self, const char* name) {
    //printf("%s %s %s\n", name, uri, elevationUri);
    return self->removeNode(name);
}

void EMSCRIPTEN_KEEPALIVE fe_addTileLayer(EarthApp* self, const char* name, const char* uri, const char* elevationUri, char* options) {
    //printf("%s %s %s\n", name, uri, elevationUri);
    XyzTileManager* mgr = self->addTileLayer(name, uri, elevationUri);

    if (options && *options) {
        //printf("%s\n", options);
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse(options);

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser options json error: %s\n", parser.get_error());
            return;
        }
        else {
            Value* minLevel = value0->get("minLevel");
            if (minLevel) {
                mgr->minLevel = minLevel->as_int();
            }

            Value* maxLevel = value0->get("maxLevel");
            if (maxLevel) {
                mgr->maxLevel = maxLevel->as_int();
            }

            Value* elevationMinLevel = value0->get("elevationMinLevel");
            if (elevationMinLevel) {
                self->getElevation()->minLevel = elevationMinLevel->as_int();
            }

            Value* elevationMaxLevel = value0->get("elevationMaxLevel");
            if (elevationMaxLevel) {
                self->getElevation()->maxLevel = elevationMaxLevel->as_int();
                //printf("elevationMaxLevel:%d\n", self->getElevation()->maxLevel);
            }

            Value* elevationScale = value0->get("elevationScale");
            if (elevationScale) {
                self->getElevation()->elevationScale = elevationScale->as_float();
            }
        }
    }
}

void EMSCRIPTEN_KEEPALIVE fe_addSkybox(EarthApp* self, int dark, float min, float max) {
    self->addSkybox(dark, min, max);
}

void EMSCRIPTEN_KEEPALIVE fe_addGeoLayer(EarthApp* self, const char* name, const char* uri, char* options) {
    GeoLayer* road = new GeoLayer(uri);
    road->setName(name);

    if (options && *options) {
        road->loadOptions(options);
    }

    self->addGeoNode(UPtr<GeoNode>(road));
}

void EMSCRIPTEN_KEEPALIVE fe_addBuildingLayer(EarthApp* self, const char* name, const char* uri, char* options) {
    Building* building = new Building(uri);
    building->setName(name);
    if (options && *options) {
        building->loadOptions(options);
    }
    self->addGeoNode(UPtr<GeoNode>(building));
}

void EMSCRIPTEN_KEEPALIVE fe_add3dtiles(EarthApp* self, const char* name, const char* uri, 
        double lng, double lat, double height, int lighting, char* options) {
    Node* layer = self->add3dtiles(name, uri, Coord2D(lng, lat), height, lighting);
    // layer->rotateX(MATH_PI / 2);
    // layer->rotateY(MATH_PI / 4);
    //layer->scale(20);
    if (options && *options) {
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse(options);

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser options json error: %s\n", parser.get_error());
            return;
        }
        else {
            Value* rotateX = value0->get("rotateX");
            if (rotateX) {
                layer->rotateX(rotateX->as_float());
            }

            Value* rotateY = value0->get("rotateY");
            if (rotateY) {
                layer->rotateY(rotateY->as_float());
            }

            Value* rotateZ = value0->get("rotateZ");
            if (rotateZ) {
                layer->rotateZ(rotateZ->as_float());
            }

            Value* scale = value0->get("scale");
            if (scale) {
                layer->scale(scale->as_float());
            }
        }
    }
}

void EMSCRIPTEN_KEEPALIVE fe_addGroundGltf(EarthApp* self, const char* name, const char* uri, 
    double lng, double lat, double height, int lighting, char* options) {
    GroundModel* model = new GroundModel(uri);
    model->setName(name);
    model->app = self;
    //model->pose.translate(0, 0, -98);
    //model->pose.scale(2);
    model->setPosition(Coord2D(lng, lat), height);
    //track->pose.rotateZ(MATH_PI);
    //model->pose.rotateX(MATH_PI / 2);
    model->lighting = lighting;
    
    if (options && *options) {
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse(options);

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser options json error: %s\n", parser.get_error());
        }
        else {
            Value* translateZ = value0->get("translateZ");
            if (translateZ) {
                model->pose.translate(0, 0, translateZ->as_float());
            }

            Value* rotateX = value0->get("rotateX");
            if (rotateX) {
                model->pose.rotateX(rotateX->as_float());
            }

            Value* rotateY = value0->get("rotateY");
            if (rotateY) {
                model->pose.rotateY(rotateY->as_float());
            }

            Value* rotateZ = value0->get("rotateZ");
            if (rotateZ) {
                model->pose.rotateZ(rotateZ->as_float());
            }

            Value* scale = value0->get("scale");
            if (scale) {
                model->pose.scale(scale->as_float());
            }

            Value* autoStickGround = value0->get("autoStickGround");
            if (autoStickGround) {
                model->autoStickGround = autoStickGround->as_int();
            }
        }
    }

    self->addGeoNode(UPtr<GeoNode>(model));
}

void EMSCRIPTEN_KEEPALIVE fe_addLight(EarthApp* self, const char* name, double lng, double lat, float r, float g, float b) {
    Vector3 sun;
    GeoCoordSys::blToXyz(Coord2D(lng, lat), sun, GeoCoordSys::earth()->getRadius() * 2);
    UPtr<Light> directionalLight = Light::createDirectional(Vector3(r, g, b));
    UPtr<Node> _directionalLightNode = Node::create(name);
    _directionalLightNode->setLight(std::move(directionalLight));
    Matrix matrix;
    Matrix::createLookAt(sun, Vector3::zero(), Vector3::unitZ(), &matrix, false);
    _directionalLightNode->setMatrix(matrix);
    self->getView()->getScene()->addNode(std::move(_directionalLightNode));
}

void EMSCRIPTEN_KEEPALIVE fe_setPosition(EarthApp* self, double lng, double lat, double zoom) {
    Coord2D coord(lng, lat);
    self->getEarthCtrl()->moveToPostion(coord);
    self->getEarthCtrl()->setZoom(zoom);
}

void EMSCRIPTEN_KEEPALIVE fe_moveTop(EarthApp* self, double lng, double lat, int time, double zoom) {
    Coord2D pos(lng, lat);
    self->getEarthCtrl()->getAnimation()->moveTo(pos.x, pos.y, time, zoom);
}

void EMSCRIPTEN_KEEPALIVE fe_zoomTo(EarthApp* self, double zoom, uint64_t time) {
    self->getEarthCtrl()->getAnimation()->zoomTo(zoom, time);
}

void EMSCRIPTEN_KEEPALIVE fe_rotateTo(EarthApp* self, float rx, float rz, uint64_t time) {
    self->getEarthCtrl()->getAnimation()->rotateTo(rx, rz, time);
}


void EMSCRIPTEN_KEEPALIVE fe_addMultiModel(EarthApp* self, const char* name, const char* uri, int lighting, char* options) {
    MultiModel* model = new MultiModel(uri);
    model->setName(name);
    //model->app = self;
    model->lighting = lighting;
    
    if (options && *options) {
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse(options);

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser options json error: %s\n", parser.get_error());
        }
        else {
        }
    }

    self->addGeoNode(UPtr<GeoNode>(model));
}

int EMSCRIPTEN_KEEPALIVE fe_updateModelInstance(EarthApp* self, const char* name, int id, 
        double lng, double lat, double height, char* options) {
    Node* node = self->getView()->getScene()->findNode(name);
    MultiModel* multiModel = dynamic_cast<MultiModel*>(node);
    //printf("@@:%p,%s\n", node, name);
    if (!multiModel) return -1;

    TrackModel* model = NULL;
    if (id == -1) {
        model = new TrackModel();
    }
    else {
        model = multiModel->get(id);
        if (!model) return -1;
    }

    std::vector<Coord2D> cpath;
    cpath.push_back(Coord2D(lng, lat));

    //track->pose.translate(i*100, 0, 0);
    //track->pose.scale(10);
    //track->pose.rotateZ(MATH_PI);
    //track->pose.rotateX(MATH_PI / 2);
    if (options && *options) {
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse(options);

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser options json error: %s\n", parser.get_error());
        }
        else {
            Value* translateZ = value0->get("translateZ");
            if (translateZ) {
                model->pose.translate(0, 0, translateZ->as_float());
            }

            Value* rotateX = value0->get("rotateX");
            if (rotateX) {
                model->pose.rotateX(rotateX->as_float());
            }

            Value* rotateY = value0->get("rotateY");
            if (rotateY) {
                model->pose.rotateY(rotateY->as_float());
            }

            Value* rotateZ = value0->get("rotateZ");
            if (rotateZ) {
                model->pose.rotateZ(rotateZ->as_float());
            }

            Value* scale = value0->get("scale");
            if (scale) {
                model->pose.scale(scale->as_float());
            }

            Value* speed = value0->get("speed");
            if (speed) {
                model->speed = speed->as_float();
            }

            Value* path = value0->get("path");
            if (path) {
                cpath.clear();
                for (auto it = path->begin(); it != path->end(); ++it) {
                    double x = it->as_float();
                    ++it;
                    double y = it->as_float();
                    cpath.push_back(Coord2D(x, y));
                }
            }

            Value* direction = value0->get("direction");
            if (direction) {
                for (auto it = path->begin(); it != path->end(); ++it) {
                    double x = it->as_float();
                    ++it;
                    double y = it->as_float();
                    ++it;
                    double z = it->as_float();
                    model->direction.x = x;
                    model->direction.y = y;
                    model->direction.z = z;
                }
            }
        }
    }

    model->setFromLonLat(cpath, height);

    if (id == -1) {
        model->start();
        return multiModel->add(UPtr<TrackModel>(model));
    }
    else {
        model->reset();
        model->start();
        return id;
    }
}

void EMSCRIPTEN_KEEPALIVE fe_removeModelInstance(EarthApp* self, const char* name, int id) {
    MultiModel* multiModel = dynamic_cast<MultiModel*>(self->getView()->getScene()->findNode(name));
    if (!multiModel) return;
    multiModel->remove(id);
}

void EMSCRIPTEN_KEEPALIVE fe_addEmptyGeoLayer(EarthApp* self, const char* name, int geotype, char* options) {
    GeoLayer* layer = new GeoLayer("");
    layer->setName(name);
    if (options && *options) {
        layer->loadOptions(options);
    }

    layer->initEmpty((GeometryType)geotype);

    self->addGeoNode(UPtr<GeoNode>(layer));
}

bool EMSCRIPTEN_KEEPALIVE fe_addGeoFeature(EarthApp* self, const char* name, int geotype, double* coords, int pointNum, char* attributes) {
    GeoLayer* layer = dynamic_cast<GeoLayer*>(self->getView()->getScene()->findNode(name));
    if (!layer) return false;

    //printf("fe_addGeoFeature:%f, %d, %s\n", coords[0], pointNum, attributes);

    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* jproperties = nullptr;
    if (attributes && *attributes) {
        //JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse((char*)attributes);
        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser geojson error: %s\n", parser.get_error());
            return false;
        }
        jproperties = value0;
    }

    if ((GeometryType)geotype == GeometryType::Point) {
        UPtr<Feature> f(new Feature());
        f->geometry.type = GeometryType::Point;
        f->geometry.coordinates.push_back(coords[0]);
        f->geometry.coordinates.push_back(coords[1]);
        f->geometry.coordinates.push_back(coords[2]);
        f->parent = layer->featureCollection.get();
        f->parseProperties(jproperties);
        layer->featureCollection->add(std::move(f));
    }
    else if ((GeometryType)geotype == GeometryType::LineString) {
        UPtr<Feature> f(new Feature());
        f->geometry.type = GeometryType::LineString;
        for (int i=0; i<pointNum; ++i) {
            f->geometry.coordinates.push_back(coords[0]);
            f->geometry.coordinates.push_back(coords[1]);
            f->geometry.coordinates.push_back(coords[2]);
            coords += 3;
        }

        GeoLine line;
        line.startPoint = 0;
        line.size = pointNum;
        f->geometry.lines.push_back(line);
        f->parent = layer->featureCollection.get();
        f->parseProperties(jproperties);
        layer->featureCollection->add(std::move(f));
    }
    else if ((GeometryType)geotype == GeometryType::Polygon) {
        UPtr<Feature> f(new Feature());
        f->geometry.type = GeometryType::Polygon;
        for (int i=0; i<pointNum; ++i) {
            f->geometry.coordinates.push_back(coords[0]);
            f->geometry.coordinates.push_back(coords[1]);
            f->geometry.coordinates.push_back(coords[2]);
            coords += 3;
        }

        GeoLine line;
        line.startPoint = 0;
        line.size = pointNum;
        f->geometry.lines.push_back(line);
        f->parent = layer->featureCollection.get();
        f->parseProperties(jproperties);
        layer->featureCollection->add(std::move(f));
    }

    layer->updateData();
    return true;
}

int EMSCRIPTEN_KEEPALIVE fe_removeGeoFeatureLike(EarthApp* self, const char* name, const char* fieldName, const char* value) {
    GeoLayer* layer = dynamic_cast<GeoLayer*>(self->getView()->getScene()->findNode(name));
    if (!layer) return 0;

    //printf("fe_removeGeoFeature: %s, %s\n", fieldName, value);

    if (layer->featureCollection.isNull()) return 0;
    int res = layer->featureCollection->removeLike(fieldName, value, false);
    if (res) {
        layer->updateData();
    }
    return res;
}

bool EMSCRIPTEN_KEEPALIVE fe_removeGeoFeatureAt(EarthApp* self, const char* name, int index) {
    GeoLayer* layer = dynamic_cast<GeoLayer*>(self->getView()->getScene()->findNode(name));
    if (!layer) return false;

    //printf("fe_removeGeoFeature: %s, %s\n", fieldName, value);

    if (layer->featureCollection.isNull()) return 0;
    layer->featureCollection->removeAt(index);
    layer->updateData();
    return true;
}

float EMSCRIPTEN_KEEPALIVE fe_getLoadProgress(EarthApp* self, const char* name) {
    GeoNode* layer = dynamic_cast<GeoNode*>(self->getView()->getScene()->findNode(name));
    if (layer) {
        return layer->getProgress();
    }

    TileLayer* tlayer = dynamic_cast<TileLayer*>(self->getView()->getScene()->findNode(name));
    if (tlayer) {
        return tlayer->getProgress();
    }

    return -1;
}

bool EMSCRIPTEN_KEEPALIVE fe_showLoadProgress(EarthApp* self, const char* name) {
    Node* node = self->getView()->getScene()->findNode(name);
    if (!node) {
        return false;
    }
    self->showLoadProgress(node);
    return true;
}

bool EMSCRIPTEN_KEEPALIVE fe_syncPick(EarthApp* self, const char* name, int x, int y, char* layerName, double* target, long* idOrIndex) {
    RayQuery query;
    Picker& picker = self->getEarthCtrl()->picker;
    Camera* _camera = self->getView()->getCamera();
    Node* _node = NULL;
    if (name && *name) {
        _node = self->getView()->getScene()->findNode(name);
    }
    else {
        _node = self->getView()->getScene()->getRootNode();
    }
    Rectangle& viewport = *self->getView()->getViewport();
    if (picker.pick(x, y, _camera, _node, viewport, query)) {

        EarthApp::PickResult result;
        if (EarthApp::getPickResult(query, result)) {
            if (layerName) {
                const char* targetName = result.layer->getName();
                strncpy(layerName, targetName, 256);
            }
            if (idOrIndex) {
                *idOrIndex = result.userId != -1 ? result.userId : result.drawableIndex;
            }
        }
        
        if (target) {
            target[0] = query.target.x;
            target[1] = query.target.y;
            target[2] = query.target.z;
        }
        return true;
    }

    return false;
}

double* EMSCRIPTEN_KEEPALIVE fe_xyzToLnglat(EarthApp* self, double x, double y, double z, double* outCoord) {
    static double staticResult[3];
    if (!outCoord) {
        outCoord = staticResult;
    }

    Coord2D coord;
    Vector3 point(x, y, z);
    double height = 0;
    GeoCoordSys::earth()->xyzToLnglat(point, coord, &height);
    outCoord[0] = coord.x;
    outCoord[1] = coord.y;
    outCoord[2] = height;

    return outCoord;
}

double* EMSCRIPTEN_KEEPALIVE fe_lnglatToXyz(EarthApp* self, double lng, double lat, double height, double* outCoord) {
    static double staticResult[3];
    if (!outCoord) {
        outCoord = staticResult;
    }

    Coord2D coord(lng, lat);
    Vector3 out;
    GeoCoordSys::earth()->lnglatToXyz(coord, height, out);
    outCoord[0] = out.x;
    outCoord[1] = out.y;
    outCoord[2] = out.z;

    return outCoord;
}

void EMSCRIPTEN_KEEPALIVE fe_setHighlight(EarthApp* self, const char* name, long idOrIndex) {
    Node* layer = (self->getView()->getScene()->findNode(name));
    if (!layer) return;
    Picker& picker = self->getEarthCtrl()->picker;
    if (GeoLayer* geoLayer = dynamic_cast<GeoLayer*>(layer)) {
        picker.clearHighlight();
        Drawable* drawable = geoLayer->getDrawable();
        if (drawable->getHighlightType() != Drawable::No) {
            std::vector<int> path = { idOrIndex };
            picker.highlight(drawable, path);
        }
    }
    else if (MultiModel* multiModel = dynamic_cast<MultiModel*>(layer)) {
        picker.clearHighlight();
        Node* node = multiModel->get(idOrIndex)->getNode();

        std::vector<Drawable*> list;
        node->getAllDrawable(list);
        std::vector<int> path;
        for (auto it = list.begin(); it != list.end(); ++it) {
            picker.highlight(*it, path);
        }
    }
    else {
        picker.clearHighlight();
        std::vector<Drawable*> list;
        layer->getAllDrawable(list);
        std::vector<int> path;
        for (auto it = list.begin(); it != list.end(); ++it) {
            picker.highlight(*it, path);
        }
    }
}

void EMSCRIPTEN_KEEPALIVE fe_clearHighlight(EarthApp* self) {
    return self->getEarthCtrl()->picker.clearHighlight();
}


}