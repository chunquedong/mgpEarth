#include "feTile/XyzTileManager.h"
#include "mgp.h"
//#include "cppfan/Profiler.h"
#include "feTile/TileData.h"
#include "feTile/TileGeom.hpp"
#include "feModel/PyramidGrid.h"
#include "feModel/GeoCoordSys.h"
#include "feElevation/Elevation.h"
#include "feTile/DataActor.h"

XyzTileManager::XyzTileManager(const std::string& uri): uri(uri), 
        #ifdef MGP_THREAD
        threadPool(nullptr),
        #endif
        viewScale(2.0) {
    maxLevel = 18;
    minLevel = 2;
    if (mgp::StringUtil::contains(uri, "bdimg")) {
        pyramid = PyramidGrid::getBD();
        minLevel = 3;
    }
    else {
        pyramid = PyramidGrid::getDefault();
    }
    Tile rootTile;
    root = TileDataPtr(new TileData(rootTile, pyramid));
}

XyzTileManager::~XyzTileManager() {
#ifdef MGP_THREAD
    if (threadPool) {
        threadPool->stop();
        SAFE_DELETE(threadPool);
    }
#endif
}

void XyzTileManager::setElevation(ElevationManager* elevation)
{
    this->elevation = elevation;
#ifdef MGP_THREAD
    if (!threadPool) {
        threadPool = new ThreadPool(1);
        threadPool->start();
    }
#endif
}

void XyzTileManager::update(Camera* camera, Rectangle* viewport, Matrix* modelMatrix, bool isSendTask) {
    TileManager::update(camera, viewport, modelMatrix, true);

    if (elevation.get()) {
        std::set<Tile> tiles;
        auto itr0 = overrviewSearchSet.begin();
        while (itr0 != overrviewSearchSet.end()) {
            TileDataPtr& tileData = itr0->second;
            if (!tileData->isReady()) {
                TileData* tileView = dynamic_cast<TileData*>(tileData.get());
                Envelope env;
                Tile tile = tileView->tileKey().tile;
                pyramid->tileEnvelope(tile, env);
                Coord2D point = env.getCenter();
                Tile elevationTile = elevation->getTileAt(point.x, point.y, tile.z-2);
                tileView->elevationTile(elevationTile);
                tiles.insert(elevationTile);
            }
            ++itr0;
        }
        auto itr = curSearchSet.begin();
        while (itr != curSearchSet.end()) {
            TileDataPtr tileData = itr->second;
            if (!tileData->isReady()) {
                TileData* tileView = dynamic_cast<TileData*>(tileData.get());
                Envelope env;
                Tile tile = tileView->tileKey().tile;
                pyramid->tileEnvelope(tile, env);
                Coord2D point = env.getCenter();
                Tile elevationTile = elevation->getTileAt(point.x, point.y, tile.z-2);
                tileView->elevationTile(elevationTile);
                tiles.insert(elevationTile);
            }
            ++itr;
        }

        if (tiles.size() > 0) {
            bool ok = false;
            if (evlevationQuery.get()) {
                ok = true;
                for (auto tile : tiles) {
                    if (!evlevationQuery->tiles.contains(tile)) {
                        ok = false;
                        break;
                    }
                }
            }
            if (!ok) {
                auto query = elevation->fromTiles(tiles);
                evlevationQuery = query;
            }
        }
        else {
            evlevationQuery.clear();
        }
    }
}

void XyzTileManager::setCachePath(const std::string& path)
{
    cachePath = path;
}

TileDataPtr XyzTileManager::getRoot() {
    return root;
}
TileDataPtr XyzTileManager::makeTileData(TileKey key) {
    TileData* tileData = new TileData(key.tile, pyramid);
    if (elevation.get()) {
        Envelope env;
        pyramid->tileEnvelope(key.tile, env);
        Coord2D center = env.getCenter();
        double height = OfflineElevation::cur()->getHeightMercator(center.x, center.y, 0);
        tileData->_approximateHeight = height;
    }
    return TileDataPtr(tileData);
}

void XyzTileManager::getChildren(TileDataPtr &tileData, std::vector<TileKey> &children) {
    TileData* tileView = dynamic_cast<TileData*>(tileData.get());
    //deep into the sub tile
    Tile sub[4];
    tileView->tile().subTile(sub);
    for (int i=0; i<4; ++i) {
        TileKey key{};
        key.tile = sub[i];
        children.push_back(key);
    }
}

bool XyzTileManager::isFitLod(TileDataPtr &tileData, Camera &camera, Rectangle &viewport, Matrix& modelMatrix) {
    TileData* tileView = dynamic_cast<TileData*>(tileData.get());

    int zoom = tileView->tile().z;
    
    if (zoom < minLevel) {
        return false;
    } else if (zoom >= maxLevel) {
        return true;
    }
    
    float viewScale = tileView->computeViewScale(camera, viewport);
    if (viewScale < this->viewScale) {
      return true;
    }
    return false;
}

bool XyzTileManager::getUri(TileKey key, std::string &uri, std::string &file) {
    Tile tile = key.tile;
    uri = this->uri;

    mgp::StringUtil::replace(uri, "{random}", std::to_string(rand() % 3));

    if (mgp::StringUtil::contains(uri, "bdimg")) {
        int halfSize = (int)pow(2.0, tile.z - 1);
        int x = -halfSize + tile.x;
        mgp::StringUtil::replace(uri, "{x}", std::to_string(x));
        int y = halfSize - 1 - tile.y;
        mgp::StringUtil::replace(uri, "{y}", std::to_string(y));
        mgp::StringUtil::replace(uri, "{z}", std::to_string(tile.z));
    }
    else if (mgp::StringUtil::contains(uri, "{q}")) {
        std::string quadKey;
        tile.toQuadKey(quadKey);
        mgp::StringUtil::replace(uri, "{q}", quadKey);
    }
    else {
        mgp::StringUtil::replace(uri, "{x}", std::to_string(tile.x));
        mgp::StringUtil::replace(uri, "{y}", std::to_string(tile.y));
        mgp::StringUtil::replace(uri, "{z}", std::to_string(tile.z));
        
        //TMS
        if (mgp::StringUtil::contains(uri, "{-y}")) {
            int y = (int)pow(2.0, tile.z) - 1 - tile.y;
            mgp::StringUtil::replace(uri, "{-y}", std::to_string(y));
        }
    }

    char buffer[256];
    // snprintf(buffer, 256, "%s/%d/%d", cachePath.c_str(), tile.z, tile.x);
    // if (!FileSystem::fileExists(buffer)) {
    //     FileSystem::mkdirs(buffer);
    // }
    snprintf(buffer, 256, "%s/%d/%d/%d.png", cachePath.c_str(), tile.z, tile.x, tile.y);
    file = buffer;
    return true;
}

void* XyzTileManager::decode(Task* task, NetResponse &res) {
    //TileKey* tile = static_cast<TileKey*>(res.id);
    Image* image = Image::createFromBuf(res.result.data(), res.result.size(), false).take();
    if (!image) return NULL;

    if (elevation.get()) {
        return image;
    }
    else {
        TileKey* tile = static_cast<TileKey*>(res.id);
        TileGeom *geometry = new TileGeom(pyramid);
        geometry->init(image, tile->tile, NULL, NULL);
        SAFE_RELEASE(image);
        return geometry;
    }
    //return image;
}

void XyzTileManager::onReceive(Task* task, NetResponse &res) {
    TileKey* tile = static_cast<TileKey*>(res.id);
    
    TileDataPtr val;
    TileData* data = dynamic_cast<TileData*>(cache._get(*tile, val).get());
    if (data && res.decodeResult) {
        if (elevation.get()) {
            data->image = UPtr<Image>((Image*)res.decodeResult);
        }
        else {
            data->geometry = UPtr<TileGeom>((TileGeom*)res.decodeResult);
        }
    }
    else if (res.decodeResult) {
        if (elevation.get()) {
            Image* image = (Image*)res.decodeResult;
            SAFE_RELEASE(image);
        }
        else {
            TileGeom* image = (TileGeom*)res.decodeResult;
            SAFE_RELEASE(image);
        }
    }

    onTaskDone(*tile);
}

bool XyzTileManager::resultChanged() {
    if (resultDirty) return true;
    if (evlevationQuery.get()) {
        return evlevationQuery->resultDirty;
    }
    return false;
}

struct MakeDataTask : public Task {
    TileDataPtr tileData;
    SPtr<ElevationQuery> evlevationQuery;
    SPtr<XyzTileManager> manager;

    virtual void run() {
        TileData* tileView = dynamic_cast<TileData*>(tileData.get());
        //check again
        if (tileView->image.get() && !tileView->geometry.get()) {
            TileGeom* geometry = new TileGeom(manager->pyramid);
            geometry->init(tileView->image.get(), tileView->tile(), &tileView->elevationTile(), evlevationQuery.get());
            tileView->geometry = UPtr<TileGeom>(geometry);
            manager->setResultDirty();
        }
    }
};

void XyzTileManager::tryInit(TileDataPtr& tileData, bool isOverview) {
    TileData* tileView = dynamic_cast<TileData*>(tileData.get());
    if (tileView->getState() == 1 && elevation.get()) {
        if (evlevationQuery.get()) {
            SPtr<TileData> elevationTileData;
            elevationTileData = evlevationQuery->tiles.get(tileView->elevationTile(), elevationTileData);
            if (elevationTileData.get() && elevationTileData->getState() == 1) {
                SPtr<MakeDataTask> task(new MakeDataTask());
                task->tileData = tileData;
                task->evlevationQuery = evlevationQuery;
                task->manager = this;
#ifdef MGP_THREAD
                threadPool->addTask(task);
#else
                task->run();
                task->done();
#endif
            }
        }
    }
}

TileDataPtr XyzTileManager::getParent(TileDataPtr t) {
    if (t->tileKey().tile.z <= this->minLevel) {
        return TileDataPtr();
    }
    return TileManager::getParent(t);
}

