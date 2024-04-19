/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef _GEOMETRY_H
#define _GEOMETRY_H

#include "feUtil/common.h"
#include "feModel/Envelope.h"
#include "jvalue.hpp"
#include "mgp_pro.h"

FE_BEGIN_NAMESPACE

enum class GeometryType {
    Unknow,
    Point,
    MultiPoint,
    LineString,
    MultiLineString,
    Polygon,
    MultiPolygon,
    GeometryCollection,
    Mix,
};

struct GeoLine {
    int startPoint;
    int size;
};

class Geometry {
public:
    Geometry(): type(GeometryType::Unknow) {}
    GeometryType type;
    std::vector<float> coordinates;
    std::vector<GeoLine> lines;
    std::vector<Geometry*> geometries;
    bool parse(jc::Value* geometry);

private:
    void parsePoint(jc::Value* jcoord);
    void parseLine(jc::Value* jcoord);
    void parsePolygon(jc::Value* jcoord);
};

class Feature {
public:
    Geometry geometry;
    std::map<std::string, std::string> properties;
    bool parse(jc::Value* feature);
};

class FeatureCollection {
public:
    GeometryType type;
    std::vector<Feature*> features;

    FeatureCollection(): type(GeometryType::Unknow) {}
    ~FeatureCollection();

    bool parse(std::string& json);
};


FE_END_NAMESPACE

#endif // _GEOMETRY_H
