#include "ElevationManager.h"
#include "mgp.h"
//#include "cppfan/Profiler.h"
#include "feTile/TileData.h"
#include "feTile/TileGeom.hpp"
#include "feModel/PyramidGrid.h"
#include "feModel/GeoCoordSys.h"


ElevationQuery::ElevationQuery(): tiles(100)
{
}

ElevationQuery::~ElevationQuery()
{
    auto mgr = manager.lock();
    if (mgr.get()) mgr->cancel(this);
}

double ElevationQuery::getHeight(double longitude, double latitude, int level)
{
    Coord2D coord(longitude, latitude);
    GeoCoordSys::earth()->toMercator(&coord);
    return getHeightMercator(coord.x, coord.y, level);
}

double ElevationQuery::getHeightMercator(double x, double y, int level) {
    auto itr = tiles.begin();
    while (itr != tiles.end()) {
        TileData* tileData = itr->second.get();

        double height = getTileHeight(tileData, x, y, NAN, level);
        if (!isnan(height)) {
            return height;
        }

        ++itr;
    }
    return 0.0;
}

double ElevationQuery::getTileHeight(Tile tile, double x, double y, double defVal, int level) {
    auto it = tiles.find(tile);
    if (it == tiles.end()) {
        return defVal;
    }
    double height = getTileHeight(it->second.get(), x, y, defVal, level);
    return height;
}

static double imageSampleOne(uint8_t* data, int w, int h, int x, int y, int ppb) {
    if (x >= w) x = w - 1;
    if (y >= h) y = h - 1;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    uint8_t* pos = data + ((w * y + x) * ppb);
    int R = pos[0];
    int G = pos[1];
    int B = pos[2];
    if (R == 6) {
        R = 1;
    }
    double height = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1);
    return height;
}

static double imageSample(Image* image, double u, double v, int level) {
    int w = image->getWidth();
    int h = image->getHeight();
    uint8_t* data = image->getData();
    int ppb = image->getFormat() == Image::RGB ? 3 : 4;

    double x = u * w;
    double y = v * h;
    int ix = (int)round(x);
    int iy = (int)round(y);

#if 0
    double value = imageSampleOne(data, w, h, ix, iy, ppb);
    return value;
#else
    int limitLevel = 11;
    if (level < limitLevel) {
        double value = imageSampleOne(data, w, h, ix, iy, ppb);
        return value;
    }
    double sumValue = 0;
    double sumWeight = 0;
    for (int i = -1; i < 2; ++i) {
        for (int j = -1; j < 2; ++j) {
            int sx = ix + i;
            int sy = iy + j;
            double dx = (sx - x);
            double dy = (sy - y);
            double disSq = dx * dx + dy * dy;
            if (disSq > 1) continue;

            double value = imageSampleOne(data, w, h, sx, sy, ppb);
            double weight = (1 - disSq) / disSq;
            sumValue += value * weight;
            sumWeight += weight;
        }
    }
    return sumValue / sumWeight;
#endif
}

double ElevationQuery::getHeightUV(TileData* tileData, double u, double v, double defVal, int level) {
    if (tileData->image.get()) {
        
        double height = imageSample(tileData->image.get(), u, v, level);
        /*if (height > 10000) {
            printf("DEBUG\n");
        }*/

        auto mgr = manager.lock();
        if (mgr.get()) {
            height *= mgr->elevationScale;
        }
        return height;
    }
    else {
        return defVal;
    }
}

double ElevationQuery::getTileHeight(TileData* tileData, double x, double y, double defVal, int level) {
    Envelope env = tileData->envelopeMercator();
    if (env.containsPoint(x, y)) {
        double u = (x - env.minX()) / env.width();
        double v = (y - env.minY()) / env.height();

        return getHeightUV(tileData, u, 1-v, defVal, level);
    }
    return defVal;
}

//void ElevationQuery::onReceive(SPtr<TileData> tileData) {
//
//}

bool ElevationQuery::isLoaded() {
    for (auto it = tiles.begin(); it != tiles.end(); ++it) {
        if (it->second->getState() == 0) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

ElevationManager::ElevationManager(const std::string& uri): uri(uri), cache(300), sendedTask(200), elevationScale(1.0)
{
    maxLevel = 7;
    minLevel = 0;
    pyramid = PyramidGrid::getDefault();

    //Tile tile;
    //SPtr<TileData> data(new TileData(tile, pyramid));
    //approximateElevation.tiles.set(tile, data);
    //data->image = UPtr<Image>(Image::create("res/fastEarth/0.png", false));
    //TileKey key;
    //key.tile = tile;
    //cache.set(key, data.dynamicCastTo<ITileData>());
    //approximateElevation.manager = this;
}

ElevationManager::~ElevationManager()
{
    auto it = sendedTask.begin();
    while (it != sendedTask.end()) {
        it->second->cancel();
        it->second->release();
        ++it;
    }
    sendedTask.clear();

    //approximateElevation._setRefCount(0);
}

SPtr<ElevationQuery> ElevationManager::fromTiles(std::set<Tile>& tiles) {
    SPtr<ElevationQuery> query(new ElevationQuery());
    
    for (auto it = tiles.begin(); it != tiles.end(); ++it) {
        Tile tile = *it;
        TileKey key;
        key.tile = tile;
        TileDataPtr data;
        if (cache.contains(key)) {
            data = cache.get(key);
            auto tileData = data.dynamicCastTo<TileData>();
            query->tiles.set(tile, tileData);
        }
        else {
            data = TileDataPtr(new TileData(tile, pyramid));
            auto tileData = data.dynamicCastTo<TileData>();
            query->tiles.set(tile, tileData);
            cache.set(key, data);
        }

        if (data->getState() == 0) {
            if (!sendedTask.contains(tile)) {
                SPtr<ElevationRequest> task = load(tile);
                task->queryCount = 1;
                if (task.get()) {
                    sendedTask.set(tile, task);
                }
            }
            else {
                SPtr<ElevationRequest> empty;
                sendedTask.get(tile, empty)->queryCount++;
            }
        }
    }

    query->manager = this;
    querys.push_back(query.get());
    return query;
}

Tile ElevationManager::getTileAt(double longitude, double latitude, int level) {
    if (level > maxLevel) {
        level = maxLevel;
    }
    if (level < minLevel) {
        level = minLevel;
    }

    Tile tile = pyramid->getTileAt(longitude, latitude, level);
    return tile;
}

Tile ElevationManager::getTileAtBL(double longitude, double latitude, int level) {
    Coord2D coord(longitude, latitude);
    GeoCoordSys::earth()->toMercator(&coord);
    return getTileAt(coord.x, coord.y, level);
}

SPtr<ElevationRequest> ElevationManager::load(Tile tile) {

    std::string url;
    std::string file;
    TileKey key;
    key.tile = tile;
    if (!getUri(key, url, file)) {
        return SPtr<ElevationRequest>();
    }
    
    ElevationRequest* client = new ElevationRequest();
    client->tileKey = tile;
    client->useCache = true;
    client->url = url;
    client->cacheFile = file;
    client->id = &client->tileKey;
    client->listener = this;
    client->send();
    
    return SPtr<ElevationRequest>(client);
}

void ElevationManager::cancel(ElevationQuery* query) {
    for (auto it = query->tiles.begin(); it != query->tiles.end(); ++it) {
        SPtr<ElevationRequest> empty;
        SPtr<ElevationRequest> req = sendedTask.get(it->first, empty);
        if (req.get()) {
            req->queryCount--;
            if (req->queryCount == 0) {
                req->cancel();
                sendedTask.remove(it->first);
            }
        }
    }
    for (auto it = querys.begin(); it != querys.end(); ++it) {
        if (*it == query) {
            querys.erase(it);
            break;
        }
    }
}

bool ElevationManager::getUri(TileKey key, std::string& uri, std::string& file) {
    Tile tile = key.tile;
    uri = this->uri;
    mgp::StringUtil::replace(uri, "{x}", std::to_string(tile.x));
    mgp::StringUtil::replace(uri, "{y}", std::to_string(tile.y));
    mgp::StringUtil::replace(uri, "{z}", std::to_string(tile.z));
    mgp::StringUtil::replace(uri, "{random}", std::to_string(rand() % 3));

    //TMS
    if (mgp::StringUtil::contains(uri, "{-y}")) {
        int y = (int)pow(2.0, tile.z) - 1 - tile.y;
        mgp::StringUtil::replace(uri, "{-y}", std::to_string(y));
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

void ElevationManager::setCachePath(const std::string& path)
{
    cachePath = path;
}

void* ElevationManager::decode(Task* task, NetResponse& res)
{
    //TileKey* tile = static_cast<TileKey*>(res.id);
    Image* image = Image::createFromBuf(res.result.data(), res.result.size(), false).take();
    if (!image) {
        image = Image::create(256, 256, Image::Format::RGBA).take();
    }
    return image;
}

void ElevationManager::onReceive(Task* t, NetResponse& res)
{
    TileKey* tile = static_cast<TileKey*>(res.id);

    TileDataPtr val;
    TileData* data = dynamic_cast<TileData*>(cache._get(*tile, val).get());
    if (data && res.decodeResult) {
        data->image = UPtr<Image>((Image*)res.decodeResult);
    }

    SPtr<ElevationRequest>  task;
    task = sendedTask.get(tile->tile, task);
    if (task.get() == nullptr || task->isCanceled()) {
        return;
    }
    sendedTask.remove(tile->tile);

    for (auto it = querys.begin(); it != querys.end(); ++it) {
        ElevationQuery* query = *it;
        if (query->tiles.contains(tile->tile)) {
            query->resultDirty = true;
        }
    }
}