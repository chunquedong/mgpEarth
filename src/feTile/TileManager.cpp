/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#include "feTile/TileManager.h"
#include "mgp.h"
//#include "cppfan/Profiler.h"
#include "feTile/DataActor.h"

FE_USING_NAMESPACE


void LRUCache::onRemove(TileKey &key, TileDataPtr &val) {
    std::lock_guard<std::mutex> guard(lock);
    toDelete.push_back(val);
}

LRUCache::~LRUCache() {
    toDelete.clear();
    clear();
}

TileManager::TileManager(): resultDirty(false), cache(300), curSearchSet(200), sendedTask(200), overrviewSearchSet(200)
{
}

TileManager::~TileManager() {
    //cache.clear();
    //curSearchSet.clear();
    auto it = sendedTask.begin();
    while (it != sendedTask.end()) {
        it->second->cancel();
        //it->second->release();
        ++it;
    }
    sendedTask.clear();
}

void TileManager::releaseCache() {
    std::lock_guard<std::mutex> guard(cache.lock);
    cache.toDelete.clear();
}

void TileManager::update(Camera* camera, Rectangle* viewport, Matrix* modelMatrix, bool isSendTask) {
    auto oldSearchSet = curSearchSet;
    curSearchSet.clear();
    overrviewSearchSet.clear();
    
    int touchedTileCount = 0;
    TileDataPtr fallback;
    TileDataPtr root = getRoot();
    
    if (root.isNull()) return;
    
    //std::vector<TileDataPtr> path;
    searchTiles(root, *camera, *viewport, *modelMatrix, touchedTileCount);
    
    //GP_DEBUG("search size %d, touched:%d\n", curSearchSet.size(), touchedTileCount);
    
    //increase cache size
    if (cache.maxSize() < touchedTileCount) {
      cache.maxSize(touchedTileCount * 2);
    }
    
    cancelTask();
    if (isSendTask) {
        sendTask(true);
        sendTask(false);
    }

    if (curSearchSet.size() != oldSearchSet.size()) {
        resultDirty = true;
    }
    else {
        for (auto itr = curSearchSet.begin(); itr != curSearchSet.end(); ++itr) {
            if (!oldSearchSet.contains(itr->first)) {
                resultDirty = true;
            }
        }
    }
}

void TileManager::getResult(std::vector<TileDataPtr>& resultList) {
    auto itr0 = overrviewSearchSet.begin();
    while (itr0 != overrviewSearchSet.end()) {
        TileDataPtr& tileView = itr0->second;
        if (!tileView->isReady()) {
            tryInit(tileView, true);
        }
        ++itr0;
    }

    HashMap<TileKey, TileDataPtr> fallbacks(50);
    auto itr = curSearchSet.begin();
    while (itr != curSearchSet.end()) {
        TileDataPtr tileView = itr->second;
        //assert(tileView->tile().z <= maxLevel && tileView->tile().z>=0);
        tryInit(tileView, false);
        
        //show all tile view
        if (tileView->isReady()) {
            tileView->setAsFallback(false);
            resultList.push_back(tileView);
        } else {
            //add fallback
            auto parent = getParent(tileView);
            while (!parent.isNull()) {
                if (parent->isReady()) {
                    fallbacks.set(parent->tileKey(), parent);
                    break;
                }
                parent = getParent(parent);
            }
        }
        ++itr;
    }

    if (curSearchSet.size() == 0) {
        _progress = 1.0;
    }
    else {
        _progress = resultList.size() / (float)curSearchSet.size();
    }

    for (auto it = fallbacks.begin(); it != fallbacks.end(); ++it) {
        TileDataPtr tileView = it->second;
        tileView->setAsFallback(true);
        resultList.push_back(tileView);
    }
    
    resultDirty = false;
}

float TileManager::getProgress() {
    return _progress;
}

bool TileManager::resultChanged() {
    return resultDirty;
}

void TileManager::searchTiles(TileDataPtr &tileView, Camera &camera, Rectangle &viewport, Matrix& modelMatrix
                  , int &count) {
    ++count;

    /*for (int i = 1; i < 16; ++i) {
        int num = 1 << i;
        if (tileView->tileKey().tile.x == 52603/ num && tileView->tileKey().tile.y == 26131/ num && tileView->tileKey().tile.z == 16-i) {
            printf("DEBUG\n");
        }
    }*/

    //test intersects
    mgp::BoundingSphere bounding = tileView->bounding();
    bounding.transform(modelMatrix);
    if (!camera.getFrustum().intersects(bounding)) {
        return;
    }
    
    //test the view scale
    if (isFitLod(tileView, camera, viewport, modelMatrix)) {
        curSearchSet.set(tileView->tileKey(), tileView);
        //assert(tileView->fallback().isNull() || tileView->fallback()->isReady());
        
        auto parent = getParent(tileView);
        if (!parent.isNull()) {
            auto parentParent = getParent(parent);
            if (!parentParent.isNull()) {
                overrviewSearchSet.set(parentParent->tileKey(), parentParent);
            }
        }

        return;
    }
    
    //deep into the sub tile
    std::vector<TileKey> children;
    getChildren(tileView, children);
    for (int i=0; i<children.size(); ++i) {
      TileKey tileKey = children[i];
      TileDataPtr subView;
      if (cache.contains(tileKey)) {
        subView = cache.get(tileKey);
      }
      else {
        subView = makeTileData(tileKey);
        subView->parent(tileView->tileKey());
        cache.set(tileKey, subView);
      }
      searchTiles(subView, camera, viewport, modelMatrix, count);
    }
}

TileDataPtr TileManager::getParent(TileDataPtr tileView) {
    TileDataPtr val;
    TileKey key = tileView->parent();
    return cache._get(key, val);
}

void TileManager::cancelTask() {
    //cancel task
    std::vector<TileKey> canceled;
    auto it = sendedTask.begin();
    while (it != sendedTask.end()) {
        TileKey key = it->first;
        if (!curSearchSet.contains(key) && !overrviewSearchSet.contains(key)) {
            it->second->cancel();
            canceled.push_back(key);
        }
        ++it;
    }
    for (int i = 0; i < canceled.size(); ++i) {
        sendedTask.remove(canceled[i]);
    }
}

void TileManager::sendTask(bool isOverview)
{
    HashMap<TileKey, TileDataPtr>* list = &curSearchSet;
    if (isOverview) {
        list = &overrviewSearchSet;
    }

    auto itr = list->begin();
    while (itr != list->end()) {
        TileKey tileKey = itr->first;
        TileDataPtr &tileView = itr->second;
    
        if (tileView->getState() == 0) {
            if (!sendedTask.contains(tileKey)) {
                SPtr<Task> task = load(tileKey);
                if (task.get()) {
                    sendedTask.set(tileKey, task);
                }
            }
        }
        ++itr;
    }

}


void TileManager::onTaskDone(TileKey key) {
    //remove from sendTask
    SPtr<Task>  task;
    task = sendedTask.get(key, task);
    if (task.get() == nullptr || task->isCanceled()) {
        return;
    }
    sendedTask.remove(key);
    resultDirty = true;

    //if (dataListener) {
    //    dataListener->sendMakeData();
    //}
}

class TileRequest : public HttpClient {
public:
    TileKey tileKey{};
};

SPtr<Task> TileManager::load(TileKey key) {

    std::string url;
    std::string file;
    if (!getUri(key, url, file)) {
        return SPtr<Task>();
    }
    
    TileRequest* client = new TileRequest();
    client->tileKey = key;
    client->useCache = true;
    client->url = url;
    client->cacheFile = file;
    client->id = &client->tileKey;
    client->listener = this;
    client->send();
    
    return SPtr<Task>(client);
}
