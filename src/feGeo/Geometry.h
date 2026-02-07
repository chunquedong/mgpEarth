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

//#include <variant>
#include <type_traits>

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

class FeatureField {
public:
    enum Type {
        Int, Float, String, Bool
    };
    Type type;
    int index;
    std::string name;
    std::string desc;
    int flags = 0;
};

struct FeatureValue {
    std::string strValue;
    union {
        int64_t intValue;
        double floatValue;
    };

    FeatureValue() : intValue(0) {}
};

class FeatureCollection;

class Feature : public mgp::Refable {
    std::vector<FeatureValue> properties;
public:
    FeatureCollection* parent = nullptr;
    Geometry geometry;

    void setFromStr(const std::string& name, const std::string& value);
    void getAsStr(const std::string& name, std::string& value);
    void getAsStr(int i, std::string& value);

    int64_t getInt(int i) {
        if (i >= this->properties.size())
            return 0;
        return properties[i].intValue;
    }
    double getFloat(int i) {
        if (i >= this->properties.size())
            return 0;
        return properties[i].floatValue;
    }
    const std::string& getStr(int i) {
        if (i >= this->properties.size())
            return "";
        return properties[i].strValue;
    }

    void setInt(int i, int64_t value) {
        if (i >= this->properties.size())
            properties.resize(i);
        properties[i].intValue = value;
    }

    void setFloat(int i, double value) {
        if (i >= this->properties.size())
            properties.resize(i);
        properties[i].floatValue = value;
    }

    void setStr(int i, const std::string& value) {
        if (i >= this->properties.size())
            properties.resize(i);
        properties[i].strValue = value;
    }

    void setValue(int i, FeatureValue& value);
    FeatureValue* getValue(int i);
    FeatureField* makeFieldValue(const std::string& name, FeatureField::Type type);
public:
    bool parse(jc::Value* feature);
    bool parseProperties(jc::Value* jproperties);
    jc::JsonNode* save(jc::JsonAllocator* allocator);
};

class FeatureCollection : public mgp::Refable {
    friend class Feature;
    std::map<std::string, int> fieldIndex;
public:
    std::string dataVersion;
    std::string crs;
    GeometryType type;
    std::vector<mgp::UPtr<Feature> > features;
    std::vector<FeatureField> fields;

    FeatureField* getField(const std::string& name);
    FeatureField* getField(int i) { return &fields[i]; }
    int getFieldCount() { return fields.size(); }
    void addField(FeatureField& field);

    void add(mgp::UPtr<Feature> f);
    Feature* get(int i) { return features[i].get(); }
    int size() { return features.size(); }

    int removeLike(const std::string& fieldName, const std::string& value, bool one = true);
    void removeAt(int index);

    FeatureCollection();
    ~FeatureCollection();

    void clear();

    bool parse(std::string& json);
    void save(std::string& json);
};


FE_END_NAMESPACE

#endif // _GEOMETRY_H
