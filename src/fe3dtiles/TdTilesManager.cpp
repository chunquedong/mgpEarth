/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#include "fe3dtiles/TdTilesManager.h"
#include "fe3dtiles/b3dm.h"

#define NODE_DIRTY_BOUNDS 2

FE_USING_NAMESPACE
PF_USING_NAMESPACE

TdTilesManager::TdTilesManager(const std::string& uri): uri(uri) {
    init();
}

TdTilesManager::~TdTilesManager() {

}

TileDataPtr TdTilesManager::getRoot() {
    TileDataPtr ptr;
    ptr = tileset.root;
    return ptr;
}

TileDataPtr TdTilesManager::makeTileData(TileKey key) {
    TileDataPtr ptr;
    ptr = static_cast<ITileData*>(key.ptr);
    return ptr;
}

void TdTilesManager::getChildren(TileDataPtr &data, std::vector<TileKey> &children) {
    TdTile* tile = dynamic_cast<TdTile*>(data.get());
    for (auto it : tile->children) {
        TileKey key{};
        key.ptr = it;
        children.push_back(key);
    }
}

bool TdTilesManager::isFitLod(TileDataPtr &data, Camera &camera, Rectangle &viewport, Matrix& modelMatrix) {
    TdTile* tile = dynamic_cast<TdTile*>(data.get());

    if (tile->children.size() == 0) {
        return true;
    }

    TdBoundingVolume bound;
    bound.set(tile->boundingVolume);
    bound.transform(modelMatrix);

    double distance = camera.getNode()->getTranslationWorld().distance(bound.center);
    double e = 0.00957 * distance;
    if (e >= tile->geometricError) {
        return true;
    }
    return false;
}

bool TdTilesManager::getUri(TileKey key, std::string &uri, std::string &file) {
    TdTile* tile = static_cast<TdTile*>(key.ptr);
    if (tile->content.uri.size() == 0) {
        return false;
    }

    uri = FileSystem::getDirectoryName(this->uri.c_str());
    uri += tile->content.uri;
    return true;
}

void TdTilesManager::init() {
    //std::string uri = FileSystem::getDirectoryName(this->uri.c_str());
    //uri += "?v=2";

    SPtr<HttpClient> client = HttpClient::create();
    client->useCache = true;
    client->url = uri;
    client->id = NULL;
    client->listener = this;
    client->send();
    //client->release();
}

//const BoundingSphere& TdTilesManager::getBoundingSphere() const {
//    if (_dirtyBits & NODE_DIRTY_BOUNDS)
//    {
//        _dirtyBits &= ~NODE_DIRTY_BOUNDS;
//
//        _bounds.set(tileset.root->boundingVolume);
//
//        _bounds.transform(getWorldMatrix());
//    }
//    return _bounds;
//}

void* TdTilesManager::decode(Task* task, NetResponse &res) {
    if (res.id == NULL) {
        return NULL;
    }

    if (FileSystem::getExtension(res.url.c_str()) == ".B3DM") {
        Stream* stream = new mgp::Buffer((uint8_t*)res.result.data(), res.result.size(), false);
        //stream->endian = Endian::Big;
        TdB3dm b3dm;
        b3dm.loadB3dm(stream);

        GltfLoader loader;
        loader.lighting = lighting;
        UPtr<Scene> scene = loader.loadFromBuf(b3dm.gltf.data(), b3dm.gltf.size());

        stream->close();
        delete stream;
        return scene.take();
    }
    return NULL;
}

void TdTilesManager::onReceive(Task* task, NetResponse &res) {
    if ((res.statusCode >= 0 && res.statusCode < 200) || res.statusCode >= 400) {
        goto end;
    }
    
    if (res.id == NULL || FileSystem::getExtension(res.url.c_str()) == ".JSON") {
        if (!res.id) {
            tileset.parse(res.result);
        }
        else {
            TileKey* tileKey = static_cast<TileKey*>(res.id);
            TdTile* tile = static_cast<TdTile*>(tileKey->ptr);
            TileLayer* layer = new TileLayer(new TdTilesManager(res.url));

            /*if (!tile->transform.isIdentity()) {
                layer->setMatrix(tile->transform);
            }*/
            tile->renderNode = UPtr<Node>(layer);
        }

        goto end;
        return;
    }
    else if (res.decodeResult) {
        TileKey* tileKey = static_cast<TileKey*>(res.id);
        TdTile* tile = static_cast<TdTile*>(tileKey->ptr);
        Scene* scene = static_cast<Scene*>(res.decodeResult);

        UPtr<Node> node = Node::create(tile->content.uri.c_str());
        scene->getRootNode()->moveChildrenTo(node.get());
        SAFE_RELEASE(scene);

        //if (!tile->transform.isIdentity()) {
        //    node->setMatrix(tile->transform);
        //}

        //modle y-up. tile bound z-up
        node->rotateX(MATH_PI / 2);

        tile->renderNode = std::move(node);
    }

end:
    if (res.id) {
        TileKey* tile = static_cast<TileKey*>(res.id);
        onTaskDone(*tile);
    }
}

mgp::Matrix* TdTilesManager::getRootTransform() {
    if (!tileset.root) {
        return nullptr;
    }
    if (!tileset.root->transform.isIdentity()) {
        return &tileset.root->transform;
    }
    return nullptr;
}
