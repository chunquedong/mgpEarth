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

    bool onPickNode(const std::string& path, Node* layer, long userId, Drawable* drawable, int drawableIndex) {
        std::string jsonstr;
        //printf("click %s\n", node->getName());
        int indexOrId = userId;
        GeoLayer* geolayer = dynamic_cast<GeoLayer*>(layer);
        if (geolayer && geolayer->featureCollection.get() && drawableIndex < geolayer->featureCollection->features.size()) {
            auto properties = geolayer->featureCollection->features[drawableIndex]->properties;

            jc::JsonAllocator allocator;
            jc::JsonNode* root = allocator.allocNode(jc::Type::Object);
            for (auto it = properties.begin(); it != properties.end(); ++it) {
                root->insert_pair(it->first.c_str(), allocator.alloc_str(it->second.c_str()));
            }
            root->reverse();
            root->to_json(jsonstr);
            indexOrId = drawableIndex;
        }
        else if (userId == -1) {
            indexOrId = drawableIndex;
        }

        return fe_appOnPickNode(this, path.c_str(), layer->getName(), indexOrId, jsonstr.c_str());
    }
};

extern "C" {

EarthApp* EMSCRIPTEN_KEEPALIVE fe_createApp(int w, int h) {
    MyEarthApp* instance = new MyEarthApp();
    Platform::run("mgpEarth", w, h);
    return instance;
}

}