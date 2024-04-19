/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#include "fe3dtiles/TdTiles.h"
#include "feUtil/common.h"
#include "jparser.hpp"

using namespace jc;
FE_USING_NAMESPACE;


TdTile::~TdTile() {
    for (auto sub : children) {
        sub->release();
    }
    children.clear();
}

void TdTileset::parse(std::string& str)
{
    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* value0 = parser.parse((char*)str.c_str());

    if (!value0 || parser.get_error()[0] != 0) {
        printf("parser 3dtiles json error: %s\n", parser.get_error());
        return;
    }

    Value* assetNode = value0->get("asset");
    if (assetNode)
    {
        Value* versionNode = assetNode->get("version");
        if (versionNode) {
            const char* version = versionNode->as_str();
        }
    }

    Value* rootNode = value0->get("root");
    if (rootNode) {
        this->root = new TdTile();
        this->root->parse(rootNode);
    }
}


TdTileset::~TdTileset() {
    SAFE_RELEASE(root);
}

#define json_get(node, name) node->get(name)

void parseBoundingVolume(Value* node, TdBoundingVolume* volume) {
    Value* box = json_get(node, "box");
    if (box) {
        int size = box->size();
        if (size != 12) return;

        auto it = box->begin();
        double v0 = it->as_float(); ++it;
        double v1 = it->as_float(); ++it;
        double v2 = it->as_float(); ++it;
        volume->center.set(v0, v1, v2);

        Vector point0;
        v0 = it->as_float(); ++it;
        v1 = it->as_float(); ++it;
        v2 = it->as_float(); ++it;
        point0.set(v0, v1, v2);

        Vector point1;
        v0 = it->as_float(); ++it;
        v1 = it->as_float(); ++it;
        v2 = it->as_float(); ++it;
        point1.set(v0, v1, v2);

        Vector point2;
        v0 = it->as_float(); ++it;
        v1 = it->as_float(); ++it;
        v2 = it->as_float(); ++it;
        point2.set(v0, v1, v2);
        
        double dis = (point0 + point1 + point2).length();
        volume->radius = dis;
        return;
    }

    Value* sphere = json_get(node, "sphere");
    if (sphere) {
        int size = sphere->size();
        if (size != 4) return;

        auto it = box->begin();
        double v0 = it->as_float(); ++it;
        double v1 = it->as_float(); ++it;
        double v2 = it->as_float(); ++it;
        volume->center.set(v0, v1, v2);

        double v3 = it->as_float(); ++it;
        volume->radius = v3;
        return;
    }

    Value* region = json_get(node, "region");
    if (region) {
        printf("unsupport BoundingVolume: region\n");
    }
}

void TdTile::parse(Value* node)
{
    Value* geometricError = json_get(node, "geometricError");
    if (geometricError) {
        this->geometricError = geometricError->as_float();
    }

    Value* boundingVolume = json_get(node, "boundingVolume");
    if (boundingVolume) {
        parseBoundingVolume(boundingVolume, &this->boundingVolume);
    }

    Value* children = json_get(node, "children");
    if (children) {
        //int size = json_size(children);
        for (auto i = children->begin(); i != children->end(); ++i) {
            Value* sub = *i;
            if (sub) {
                TdTile* tile = new TdTile();
                tile->parse(sub);
                this->children.push_back(tile);
            }
        }
    }

    Value* content = json_get(node, "content");
    if (content) {
        Value* uri = json_get(content, "url");
        if (uri) {
            const char* str = uri->as_str();
            this->content.uri = std::string(str);
        }
        else {
            uri = json_get(content, "uri");
            if (uri) {
                const char* str = uri->as_str();
                this->content.uri = std::string(str);
            }
        }
        Value* boundingVolume = json_get(content, "boundingVolume");
        if (boundingVolume) {
            parseBoundingVolume(boundingVolume, &this->content.boundingVolume);
        }
    }

    Value* refine = json_get(node, "refine");
    if (refine) {
        const char* str = refine->as_str();
        std::string refine = std::string(str);
        //json_free(str);

        if (refine == "REPLACE") {
            this->refine = Refinement::REPLACE;
        }
        else if (refine == "ADD") {
            this->refine = Refinement::ADD;
        }
    }
}

mgp::BoundingSphere& TdTile::bounding() {
    return boundingVolume;
}

int TdTile::getState() {
    if (!renderNode.isNull()) {
        return 2;
    }
    return 0;
}

mgp::Node* TdTile::getNode() {
    return renderNode.get();
}

TileKey TdTile::tileKey() {
    TileKey key{};
    key.ptr = this;
    return key;
}

