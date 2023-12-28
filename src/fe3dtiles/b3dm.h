/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef _3DTILE_B3DM_H
#define _3DTILE_B3DM_H

#include "feUtil/common.h"
#include "mgp.h"


FE_BEGIN_NAMESPACE

struct TdTilesData {
};

struct TdB3dm : public TdTilesData {
    std::string gltf;
    mgp::Vector3 rtcCenter;


    bool loadB3dm(mgp::Stream* data);
};


FE_END_NAMESPACE

#endif
