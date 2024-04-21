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

void EMSCRIPTEN_KEEPALIVE fe_add3dtiles(EarthApp* self, const char* name, const char* uri, float lng, float lat, float height, bool lighting, char* options) {
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

void EMSCRIPTEN_KEEPALIVE fe_addGroundGltf(EarthApp* self, const char* name, const char* uri, float lng, float lat, float height, bool lighting, char* options) {
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

void EMSCRIPTEN_KEEPALIVE fe_addLight(EarthApp* self, const char* name, float lng, float lat, float r, float g, float b) {
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

void EMSCRIPTEN_KEEPALIVE fe_setPosition(EarthApp* self, float lng, float lat, float zoom) {
    Coord2D coord(lng, lat);
    self->getEarthCtrl()->moveToPostion(coord);
    self->getEarthCtrl()->setZoom(zoom);
}

void EMSCRIPTEN_KEEPALIVE fe_moveTop(EarthApp* self, float lng, float lat, int time, float zoom) {
    Coord2D pos(lng, lat);
    self->getEarthCtrl()->getAnimation()->moveTo(pos.x, pos.y, time, zoom);
}

void EMSCRIPTEN_KEEPALIVE fe_zoomTo(EarthApp* self, float zoom, uint64_t time) {
    self->getEarthCtrl()->getAnimation()->zoomTo(zoom, time);
}

void EMSCRIPTEN_KEEPALIVE fe_rotateTo(EarthApp* self, float rx, float rz, uint64_t time) {
    self->getEarthCtrl()->getAnimation()->rotateTo(rx, rz, time);
}


void EMSCRIPTEN_KEEPALIVE fe_addMultiModel(EarthApp* self, const char* name, const char* uri, bool lighting, char* options) {
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

int EMSCRIPTEN_KEEPALIVE fe_updateModelInstance(EarthApp* self, const char* name, int id, float lng, float lat, float height, char* options) {
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
                    float x = it->as_float();
                    ++it;
                    float y = it->as_float();
                    cpath.push_back(Coord2D(x, y));
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

bool EMSCRIPTEN_KEEPALIVE fe_addGeoFeature(EarthApp* self, const char* name, int geotype, float* coords, int pointNum, char* attributes) {
    GeoLayer* layer = dynamic_cast<GeoLayer*>(self->getView()->getScene()->findNode(name));
    if (!layer) return false;

    //printf("fe_addGeoFeature:%f, %d, %s\n", coords[0], pointNum, attributes);

    std::map<std::string, std::string> properties_map;
    if (attributes && *attributes) {
        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse((char*)attributes);

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser geojson error: %s\n", parser.get_error());
            return false;
        }

        Value* properties = value0;
        for (auto i = properties->begin(); i != properties->end(); ++i) {
            std::string key = i.get_name();
            std::string val;
            if (i->type() == Type::String) {
                val = i->as_str();
            }
            else {
                i->to_json(val);
            }
            properties_map[key] = val;
        }
    }

    if ((GeometryType)geotype == GeometryType::Point) {
        Feature* f = new Feature();
        f->geometry.type = GeometryType::Point;
        f->geometry.coordinates.push_back(coords[0]);
        f->geometry.coordinates.push_back(coords[1]);
        f->geometry.coordinates.push_back(coords[2]);
        f->properties.swap(properties_map);
        layer->featureCollection->add(f);
    }
    else if ((GeometryType)geotype == GeometryType::LineString) {
        Feature* f = new Feature();
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
        f->properties.swap(properties_map);
        layer->featureCollection->add(f);
    }
    else if ((GeometryType)geotype == GeometryType::Polygon) {
        Feature* f = new Feature();
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
        f->properties.swap(properties_map);
        layer->featureCollection->add(f);
    }

    layer->updateData();
    return true;
}

int EMSCRIPTEN_KEEPALIVE fe_removeGeoFeature(EarthApp* self, const char* name, const char* fieldName, const char* value) {
    GeoLayer* layer = dynamic_cast<GeoLayer*>(self->getView()->getScene()->findNode(name));
    if (!layer) return 0;

    //printf("fe_removeGeoFeature: %s, %s\n", fieldName, value);

    if (layer->featureCollection.isNull()) return 0;
    int res = layer->featureCollection->remove(fieldName, value, false);
    if (res) {
        layer->updateData();
    }
    return res;
}

}