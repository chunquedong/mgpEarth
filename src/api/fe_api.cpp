/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#include "FastEarth.h"
#include "mgp_pro.h"
#include "jparser.hpp"

#include <stdio.h>
#include <emscripten.h>

using namespace jc;

EM_JS(void, fe_appInitialize, (void* self), {
    fe_onInitialize(self);
});

EM_JS(bool, fe_appOnPickNode, (void* self, const char* path, const char* name, int index, const char* properties), {
    return fe_onPickNode(self, path, name, index, properties);
});

static jc::JsonNode* json_new_a(jc::JsonAllocator& allocator, const char* s) {
    jc::JsonNode* value = allocator.allocNode(jc::Type::String);
    value->set_str(allocator.strdup(s));
    return value;
}

class MyEarthApp : public EarthApp {
    void initialize() {
        showFps(false);
        EarthApp::initialize();
        fe_appInitialize(this);
    }

    bool onPickNode(const std::string& path, Node* layer, Drawable* drawable, int index) {
        std::string jsonstr;
        //printf("click %s\n", node->getName());
        GeoLayer* geolayer = dynamic_cast<GeoLayer*>(layer);
        if (geolayer && geolayer->featureCollection && index < geolayer->featureCollection->features.size()) {
            auto properties = geolayer->featureCollection->features[index]->properties;

            jc::JsonAllocator allocator;
            jc::JsonNode* root = allocator.allocNode(jc::Type::Object);
            for (auto it = properties.begin(); it != properties.end(); ++it) {
                root->insert_pair(it->first.c_str(), json_new_a(allocator, it->second.c_str()));
            }
            root->reverse();
            root->to_json(jsonstr);
        }

        return fe_appOnPickNode(this, path.c_str(), layer->getName(), index, jsonstr.c_str());
    }
};

extern "C" {

EMSCRIPTEN_KEEPALIVE
MyEarthApp* fe_createApp(int w, int h) {
    MyEarthApp* instance = new MyEarthApp();
    Platform::run("fastEarth", w, h);
    return instance;
}

EMSCRIPTEN_KEEPALIVE
void fe_showFps(MyEarthApp* self, bool show) {
    self->showFps(show);
}

EMSCRIPTEN_KEEPALIVE
bool fe_removeNode(MyEarthApp* self, const char* name) {
    //printf("%s %s %s\n", name, uri, elevationUri);
    return self->removeNode(name);
}

EMSCRIPTEN_KEEPALIVE
void fe_addTileLayer(MyEarthApp* self, const char* name, const char* uri, const char* elevationUri, char* options) {
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

EMSCRIPTEN_KEEPALIVE
void fe_addSkybox(MyEarthApp* self, int dark, float min, float max) {
    self->addSkybox(dark, min, max);
}

EMSCRIPTEN_KEEPALIVE
void fe_addGeoLayer(MyEarthApp* self, const char* name, const char* uri, char* options) {
    GeoLayer* road = new GeoLayer(uri);
    road->setName(name);

    if (options && *options) {
        road->loadOptions(options);
    }

    self->addGeoNode(UPtr<GeoNode>(road));
}

EMSCRIPTEN_KEEPALIVE
void fe_addBuildingLayer(MyEarthApp* self, const char* name, const char* uri, char* options) {
    Building* building = new Building(uri);
    building->setName(name);
    if (options && *options) {
        building->loadOptions(options);
    }
    self->addGeoNode(UPtr<GeoNode>(building));
}

EMSCRIPTEN_KEEPALIVE
void fe_add3dtiles(MyEarthApp* self, const char* name, const char* uri, float lng, float lat, float height, bool lighting, char* options) {
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

EMSCRIPTEN_KEEPALIVE
void fe_addGroundGltf(MyEarthApp* self, const char* name, const char* uri, float lng, float lat, float height, bool lighting, char* options) {
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

EMSCRIPTEN_KEEPALIVE
void fe_addLight(MyEarthApp* self, const char* name, float lng, float lat, float r, float g, float b) {
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

EMSCRIPTEN_KEEPALIVE
void fe_setPosition(MyEarthApp* self, float lng, float lat, float zoom) {
    Coord2D coord(lng, lat);
    self->getEarthCtrl()->moveToPostion(coord);
    self->getEarthCtrl()->setZoom(zoom);
}

EMSCRIPTEN_KEEPALIVE
void fe_moveTop(MyEarthApp* self, float lng, float lat, int time, float zoom) {
    Coord2D pos(lng, lat);
    self->getEarthCtrl()->getAnimation()->moveTo(pos.x, pos.y, time, zoom);
}

EMSCRIPTEN_KEEPALIVE
void fe_zoomTo(MyEarthApp* self, float zoom, uint64_t time) {
    self->getEarthCtrl()->getAnimation()->zoomTo(zoom, time);
}

EMSCRIPTEN_KEEPALIVE
void fe_rotateTo(MyEarthApp* self, float rx, float rz, uint64_t time) {
    self->getEarthCtrl()->getAnimation()->rotateTo(rx, rz, time);
}


EMSCRIPTEN_KEEPALIVE
void fe_addMultiModel(MyEarthApp* self, const char* name, const char* uri, bool lighting, char* options) {
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

EMSCRIPTEN_KEEPALIVE
int fe_updateModelInstance(MyEarthApp* self, const char* name, int id, float lng, float lat, float height, char* options) {
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

EMSCRIPTEN_KEEPALIVE
void fe_removeModelInstance(MyEarthApp* self, const char* name, int id) {
    MultiModel* multiModel = dynamic_cast<MultiModel*>(self->getView()->getScene()->findNode(name));
    if (!multiModel) return;
    multiModel->remove(id);
}

}