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
    std::vector<double> coordinates;
    std::vector<GeoLine> lines;
    std::vector<Geometry*> geometries;

    bool parse(jc::Value* geometry);
    jc::JsonNode* save(jc::JsonAllocator* allocator);

    void getPoint(mgp::Vector3& point, int i);
    int getPointCount();

private:
    void parsePoint(jc::Value* jcoord);
    void parseLine(jc::Value* jcoord);
    void parsePolygon(jc::Value* jcoord);

    jc::JsonNode* savePoint(int i, jc::JsonAllocator* allocator);
    jc::JsonNode* saveLine(int i, jc::JsonAllocator* allocator);
    jc::JsonNode* savePolygon(jc::JsonAllocator* allocator);
    jc::JsonNode* saveGeomertyCollection(jc::JsonAllocator* allocater);
};

class Feature {
public:
    Geometry geometry;
    std::map<std::string, std::string> properties;
    bool parse(jc::Value* feature);
    jc::JsonNode* save(jc::JsonAllocator* allocator);
};

class FeatureCollection : public mgp::Refable {
public:
    GeometryType type;
    std::vector<Feature*> features;

    void add(Feature* f);
    int remove(const std::string& fieldName, const std::string& value, bool one = true);
    void removeAt(int index);

    FeatureCollection(): type(GeometryType::Unknow) {}
    ~FeatureCollection();

    bool parse(std::string& json);
    void save(std::string& json);
};


FE_END_NAMESPACE

#endif // _GEOMETRY_H
