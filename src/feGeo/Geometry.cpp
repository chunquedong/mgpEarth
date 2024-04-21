/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#include "Geometry.h"
#include "jparser.hpp"

using namespace jc;
FE_USING_NAMESPACE

void FeatureCollection::add(Feature* f) {
    if (this->type != GeometryType::Mix && f->geometry.type != this->type) {
        GP_ERROR("geometry type error:%d", f->geometry.type);
    }
    features.push_back(f);
}

int FeatureCollection::remove(const std::string& fieldName, const std::string& value, bool one) {
    int n = 0;
    for (auto it = features.begin(); it != features.end();) {
        Feature* f = *it;
        auto found = f->properties.find(fieldName);
        if (found == f->properties.end()) {
            ++it;
            continue;
        }
        if (found->second == value) {
            it = features.erase(it);
            delete f;
            ++n;
            if (one) {
                break;
            }
            continue;
        }
        ++it;
    }
    return n;
}

FeatureCollection::~FeatureCollection() {
    for (auto it = features.begin(); it != features.end(); ++it) {
        delete *it;
    }
    features.clear();
}

bool FeatureCollection::parse(std::string& json) {
    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* value0 = parser.parse((char*)json.c_str());

    if (!value0 || parser.get_error()[0] != 0) {
        printf("parser geojson error: %s\n", parser.get_error());
        return false;
    }

    Value* type = value0->get("type");
    if (!type || type->type() != Type::String || strcmp(type->as_str(), "FeatureCollection") != 0)
    {
        printf("expected GEOJSON format\n");
        return false;
    }

    Value* features = value0->get("features");
    if (!features) return false;

    for (auto i = features->begin(); i != features->end(); ++i) {
        Value* feature = *i;
        Feature* f = new Feature();
        bool ok = f->parse(feature);
        if (ok) {
            this->features.emplace_back(f);
            if (this->type == GeometryType::Unknow) {
                this->type = f->geometry.type;
            }
            else if (this->type != f->geometry.type) {
                this->type = GeometryType::Mix;
            }
        }
        else {
            delete f;
        }
    }
    return true;
}

bool Feature::parse(Value* feature) {
    Value *geometry = feature->get("geometry");
    if (!geometry) return false;
    this->geometry.parse(geometry);

    Value *properties = feature->get("properties");
    for (auto i = properties->begin(); i != properties->end(); ++i) {
        std::string key = i.get_name();
        std::string val;
        if (i->type() == Type::String) {
            val = i->as_str();
        }
        else {
            i->to_json(val);
        }
        this->properties[key] = val;
    }
    return true;
}

void Geometry::parsePoint(Value* jcoord) {
    auto c = jcoord->begin();
    coordinates.push_back(c->as_float());
    ++c;
    coordinates.push_back(c->as_float());
    ++c;
    if (c != jcoord->end()) {
        coordinates.push_back(c->as_float());
    }
    else {
        coordinates.push_back(0);
    }
}

void Geometry::parseLine(Value* jcoord) {
    GeoLine line;
    line.startPoint = coordinates.size()/3;
    for (auto i = jcoord->begin(); i != jcoord->end(); ++i) {
        parsePoint(*i);
    }
    line.size = coordinates.size()/3 - line.startPoint;
    lines.push_back(line);
}

void Geometry::parsePolygon(Value* jcoord) {
    for (auto i = jcoord->begin(); i != jcoord->end(); ++i) {
        parseLine(*i);
    }
}

bool Geometry::parse(Value* geometry) {
    Value* type = geometry->get("type");
    if (!type) {
        return false;
    }
    const char* typeStr = type->as_str();
    if (strcmp(typeStr, "Point") == 0) {
        this->type = GeometryType::Point;
        Value* jcoord = geometry->get("coordinates");
        parsePoint(jcoord);
    }
    else if (strcmp(typeStr, "LineString") == 0) {
        this->type = GeometryType::LineString;
        Value* jcoord = geometry->get("coordinates");
        parseLine(jcoord);
    }
    else if (strcmp(typeStr, "Polygon") == 0) {
        this->type = GeometryType::Polygon;
        Value* jcoord = geometry->get("coordinates");
        parsePolygon(jcoord);
    }
    else if (strcmp(typeStr, "GeometryCollection") == 0) {
        this->type = GeometryType::GeometryCollection;
        Value* jcoord = geometry->get("coordinates");
        for (auto i = jcoord->begin(); i != jcoord->end(); ++i) {
            Geometry *geom = new Geometry();
            geom->parse(*i);
            geometries.push_back(geom);
        }
    }
    else if (strcmp(typeStr, "MultiPoint") == 0) {
        this->type = GeometryType::MultiPoint;
        Value* jcoord = geometry->get("coordinates");
        parseLine(jcoord);
    }
    else if (strcmp(typeStr, "MultiLineString") == 0) {
        this->type = GeometryType::MultiLineString;
        Value* jcoord = geometry->get("coordinates");
        parsePolygon(jcoord);
    }
    else if (strcmp(typeStr, "MultiPolygon") == 0) {
        this->type = GeometryType::MultiPolygon;
        Value* jcoord = geometry->get("coordinates");
        for (auto i = jcoord->begin(); i != jcoord->end(); ++i) {
            Geometry *geom = new Geometry();
            geom->parsePolygon(*i);
            geom->type = GeometryType::Polygon;
            geometries.push_back(geom);
        }
    }
    else {
        printf("UNKNOW geo type: %s\n", typeStr);
        return false;
    }
    return true;
}
