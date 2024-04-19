/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef _BUILDING_H
#define _BUILDING_H

#include "feUtil/common.h"
#include "mgp_pro.h"
#include "feModel/Envelope.h"
#include "GeoLayer.h"

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE

class Building : public GeoNode {
public:
    Vector4 color;
    float heightScale = 4.0;

    Building(const char* uri);
    ~Building();

    void* decodeFile(const char* path, NetResponse& lastRes, MultiRequest* req) override;
    bool loadOptions(char* json_str);
};

FE_END_NAMESPACE

#endif // _BUILDING_H