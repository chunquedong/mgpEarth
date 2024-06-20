/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef _GEO_NODE_H
#define _GEO_NODE_H

#include "feUtil/common.h"
#include "mgp_pro.h"
#include <float.h>

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE

class ElevationManager;
class EarthCtrl;

class GeoNode : public NetModel {
protected:
    bool isVisiable = false;
public:
    EarthCtrl* ctrl = NULL;
    double minDis = 0;
    double maxDis = FLT_MAX;

    GeoNode(const char* uri);
    void update(float elapsedTime) override;
};


class GltfNode : public GeoNode {
public:
    int lighting = 0;
    GltfNode(const char* uri);
    
    virtual void* decodeFile(const char* path, NetResponse& lastRes, MultiRequest* req) override;
    void scanAttachedFiles(NetResponse &res, std::vector<std::string>& urls, MultiRequest* req) override;

};

FE_END_NAMESPACE

#endif // _GEO_NODE_H
