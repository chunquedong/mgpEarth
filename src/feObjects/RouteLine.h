/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef _ROUTELINE_H
#define _ROUTELINE_H

#include "feUtil/common.h"
#include "mgp_pro.h"
#include "feModel/Envelope.h"
#include "GeoNode.h"

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE

class RouteLine : public GeoNode {
    int updateLevel = -1;
    MeshLine* line = NULL;
public:
    double height = 0;
    LineStyle style;
    std::vector<Coord2D> path;
    RouteLine();
    void update(float elapsedTime) override;
};


FE_END_NAMESPACE

#endif // _ROUTELINE_H
