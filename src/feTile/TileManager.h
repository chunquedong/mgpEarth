/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include "feModel/Tile.h"
#include "mgp.h"

FE_BEGIN_NAMESPACE

union TileKey {
    Tile tile;
    void* ptr;
    uint64_t id;
    
    TileKey() {
        memset(this, 0, sizeof(TileKey));
    };
    
    bool operator== (const TileKey &other) const {
        return tile == other.tile;
    }

    size_t hashCode() const {
        return tile.hashCode();
    }
};
FE_END_NAMESPACE
FE_USING_NAMESPACE
namespace std {
  template <> struct hash<TileKey> {
    size_t operator()(const TileKey &key) const {
      return key.hashCode();
    }
  };
}
FE_BEGIN_NAMESPACE


class ITileData : public Refable {
public:
    //CF_FIELD_REF(SPtr<ITileData>, fallback)
    CF_FIELD(TileKey, parent)
public:
    virtual mgp::BoundingSphere& bounding() = 0;
    bool isReady() { return getState() == 2; }
    virtual int getState() = 0;
    virtual mgp::Node* getNode() = 0;
    virtual TileKey tileKey() = 0;
    virtual void setAsFallback(bool isFallback) {}
};
typedef SPtr<ITileData> TileDataPtr;


class LRUCache : public Cache<TileKey, TileDataPtr> {
public:
    std::mutex lock;
    std::vector<TileDataPtr> toDelete;
    LRUCache(long maxSize) : Cache(maxSize) {}
    ~LRUCache();
    virtual void onRemove(TileKey &key, TileDataPtr &val) override;
};

class DataActor;

class TileManager : public Refable, public NetListener {
protected:
    LRUCache cache;
    HashMap<TileKey, TileDataPtr> curSearchSet;
    HashMap<TileKey, TileDataPtr> overrviewSearchSet;
private:
    //send task container for cancle
    HashMap<TileKey, SPtr<Task> > sendedTask;
    Cache<TileKey, int> errorTask;
    float _progress = 0;
protected:
    bool resultDirty;
public:
    //DataActor* dataListener;
public:
    TileManager();
    virtual ~TileManager();
    virtual void update(Camera* camera, Rectangle* viewport, Matrix* modelMatrix, bool isSendTask = true);
    void getResult(std::vector<TileDataPtr>& list);
    virtual bool resultChanged();
    void setResultDirty() { resultDirty = true; }

    void releaseCache();

    float getProgress();
private:
    void searchTiles(TileDataPtr &tileView, Camera &camera, Rectangle &viewport, Matrix& modelMatrix
                  , int &count);
protected:
    void sendTask(bool isOverview);
    void cancelTask();
protected:
    virtual void onTaskDone(TileKey key);
    virtual SPtr<Task> load(TileKey key);
    virtual TileDataPtr getParent(TileDataPtr t);
protected:
    virtual TileDataPtr getRoot() = 0;
    virtual TileDataPtr makeTileData(TileKey key) = 0;
    virtual void tryInit(TileDataPtr &data, bool isOverview) {}
    virtual void getChildren(TileDataPtr &data, std::vector<TileKey> &children) = 0;
    virtual bool isFitLod(TileDataPtr &data, Camera &camera, Rectangle &viewport, Matrix& modelMatrix) = 0;
    
    virtual bool getUri(TileKey key, std::string &uri, std::string &file) = 0;

    virtual void* decode(Task* task, NetResponse &res) = 0;
    virtual void onReceive(Task* task, NetResponse &res) = 0;
};

FE_END_NAMESPACE

#endif
