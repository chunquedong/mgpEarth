/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef _SKY_BOX_H_
#define _SKY_BOX_H_

#include "feUtil/common.h"
#include "mgp_pro.h"
#include "feModel/Envelope.h"
#include "GeoNode.h"

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE

class SkyBox : public GeoNode {
public:
    bool followCamera = true;
    std::vector<std::string> faces;
    SkyBox(std::vector<std::string>& faces);
    void load() override;
};


FE_END_NAMESPACE

#endif // _SKY_BOX_H_