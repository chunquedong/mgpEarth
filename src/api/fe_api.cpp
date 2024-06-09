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
#include <emscripten.h>

using namespace jc;

EM_JS(void, fe_appInitialize, (void* self), {
    fe_onInitialize(self);
});

EM_JS(bool, fe_appOnPickNode, (void* self, const char* path, const char* name, int indexOrId, const char* properties), {
    return fe_onPickNode(self, path, name, indexOrId, properties);
});

class MyEarthApp : public EarthApp {
    void initialize() {
        showFps(false);
        EarthApp::initialize();
        fe_appInitialize(this);
    }

    bool onPickNode(PickResult& pickResult) {
        std::string jsonstr;
        //printf("click %s\n", node->getName());
        int indexOrId = pickResult.userId != -1 ? pickResult.userId : pickResult.drawableIndex;

        GeoLayer* geolayer = dynamic_cast<GeoLayer*>(pickResult.layer);
        if (geolayer && geolayer->featureCollection.get() && indexOrId < geolayer->featureCollection->features.size()) {
            auto properties = geolayer->featureCollection->features[indexOrId]->properties;

            jc::JsonAllocator allocator;
            jc::JsonNode* root = allocator.allocNode(jc::Type::Object);
            for (auto it = properties.begin(); it != properties.end(); ++it) {
                root->insert_pair(it->first.c_str(), allocator.alloc_str(it->second.c_str()));
            }
            root->reverse();
            root->to_json(jsonstr);
        }

        return fe_appOnPickNode(this, pickResult.path.c_str(), pickResult.layer->getName(), indexOrId, jsonstr.c_str());
    }
};

extern "C" {

EarthApp* EMSCRIPTEN_KEEPALIVE fe_createApp(int w, int h) {
    MyEarthApp* instance = new MyEarthApp();
    Platform::run("mgpEarth", w, h);
    return instance;
}

}