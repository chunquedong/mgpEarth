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

FeatureField* FeatureCollection::getField(const std::string& name)
{
    auto it = fieldIndex.find(name);
    if (it == fieldIndex.end()) {
        return nullptr;
    }
    return &fields[it->second];
}

void FeatureCollection::add(mgp::UPtr<Feature> f) {
    f->parent = this;
    features.push_back(std::move(f));
}

int FeatureCollection::removeLike(const std::string& fieldName, const std::string& value, bool one) {
    int n = 0;
    for (auto it = features.begin(); it != features.end();) {
        Feature* f = it->get();
        std::string fvalue;
        f->getAsStr(fieldName, fvalue);
        if (fvalue == value) {
            it = features.erase(it);
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

void FeatureCollection::removeAt(int index) {
    GP_ASSERT(index >= 0);
    GP_ASSERT(index < features.size());
    features.erase(features.begin()+index);
}

FeatureCollection::~FeatureCollection() {
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
        mgp::UPtr<Feature> f(new Feature());
        f->parent = this;
        bool ok = f->parse(feature);
        if (ok) {
            if (this->type == GeometryType::Unknow) {
                this->type = f->geometry.type;
            }
            else if (this->type != f->geometry.type) {
                this->type = GeometryType::Mix;
            }
            this->add(std::move(f));
        }
    }
    return true;
}

void FeatureCollection::save(std::string& json)
{
    JsonAllocator allocator;
    jc::JsonNode* root = allocator.allocNode(jc::Type::Object);

    JsonNode* fs = allocator.allocNode(jc::Type::Array);
    for (auto it = features.begin(); it != features.end(); ++it) {
        auto jn = (*it)->save(&allocator);
        fs->append(jn);
    }
    fs->reverse();
    root->insert_pair("features", fs);
    root->insert_pair("type", allocator.alloc_str("FeatureCollection"));
    root->to_json(json);
}

void FeatureCollection::addField(FeatureField& field)
{
    int index = this->fields.size();
    field.index = index;
    fieldIndex[field.name] = index;
    this->fields.push_back(field);
}

void Feature::setFromStr(const std::string& name, const std::string& value)
{
    auto it = parent->fieldIndex.find(name);
    if (it == parent->fieldIndex.end()) {
        return;
    }
    int i = it->second;
    FeatureField::Type type = parent->fields.at(i).type;

    switch (type)
    {
    case mgpEarth::FeatureField::Int:
        this->properties[i].intValue = atoll(value.c_str());
        break;
    case mgpEarth::FeatureField::Bool:
        this->properties[i].intValue = (value == "true");
        break;
    case mgpEarth::FeatureField::Float:
        this->properties[i].floatValue = atof(value.c_str());
        break;
    case mgpEarth::FeatureField::String:
        this->properties[i].strValue = value;
        break;
    default:
        break;
    }
}

void Feature::getAsStr(const std::string& name, std::string& value)
{
    auto it = parent->fieldIndex.find(name);
    if (it == parent->fieldIndex.end()) {
        return;
    }
    int i = it->second;
    getAsStr(i, value);
}

void Feature::getAsStr(int i, std::string& value) {
    FeatureField::Type type = parent->fields.at(i).type;

    switch (type)
    {
    case mgpEarth::FeatureField::Int:
        value = std::to_string(this->properties[i].intValue);
        break;
    case mgpEarth::FeatureField::Bool:
        value = this->properties[i].intValue ? "true" : "false";
        break;
    case mgpEarth::FeatureField::Float:
        value = std::to_string(this->properties[i].floatValue);
        break;
    case mgpEarth::FeatureField::String:
        value = this->properties[i].strValue;
        break;
    default:
        break;
    }
}

bool Feature::parseProperties(jc::Value* jproperties) {
    if (!jproperties || jproperties->type() != jc::Type::Object) {
        return false;
    }

    for (auto i = jproperties->begin(); i != jproperties->end(); ++i) {
        std::string key = i.get_name();

        FeatureField::Type type;
        switch (i->type())
        {
        case jc::Type::Integer:
            type = FeatureField::Int;
            break;
        case jc::Type::Boolean:
            type = FeatureField::Bool;
            break;
        case jc::Type::Float:
            type = FeatureField::Float;
            break;
        default:
            type = FeatureField::String;
            break;
        }
        FeatureField* field = makeFieldValue(key, type);

        switch (field->type)
        {
        case mgpEarth::FeatureField::Int:
            this->properties[field->index].intValue = i->as_int();
            break;
        case mgpEarth::FeatureField::Float:
            this->properties[field->index].floatValue = i->as_float();
            break;
        case mgpEarth::FeatureField::String:
            if (i->type() == Type::String) {
                this->properties[field->index].strValue = i->as_str();
            }
            else {
                i->to_json(this->properties[field->index].strValue);
            }
            break;
        default:
            break;
        }
    }
    return true;
}

bool Feature::parse(Value* feature) {
    Value *geometry = feature->get("geometry");
    if (!geometry) return false;
    this->geometry.parse(geometry);

    Value *jproperties = feature->get("properties");
    return parseProperties(jproperties);
}

void Feature::setValue(int i, FeatureValue& value) {
    properties.at(i) = value;
}
FeatureValue* Feature::getValue(int i) {
    return &properties.at(i);
}

FeatureField* Feature::makeFieldValue(const std::string& name, FeatureField::Type type)
{
    FeatureField* field = parent->getField(name);
    if (!field) {
        FeatureField nfield;
        nfield.name = name;
        nfield.type = type;
        parent->addField(nfield);
        field = parent->getField(name);
    }
    else if (field->type != type) {
        if (field->type == FeatureField::Type::Float && type == FeatureField::Type::Int) {
            //pass
        }
        else if (type == FeatureField::Type::Float && field->type == FeatureField::Type::Int) {
            //convert from int to float
            for (auto& feature : parent->features) {
                if (feature.get()) {
                    int64_t val = feature->properties[field->index].intValue;
                    feature->properties[field->index].floatValue = val;
                }
            }
            field->type = type;
        }
        else {
            printf("ERROR: type error %d -> %d\n", field->type, type);
        }
    }

    if (this->properties.size() != parent->fields.size()) {
        this->properties.resize(parent->fields.size());
    }
    return field;
}

jc::JsonNode* Feature::save(jc::JsonAllocator* allocator)
{
    jc::JsonNode* jproperties = allocator->allocNode(jc::Type::Object);
    for (int i = 0; i < parent->getFieldCount(); ++i) {
        FeatureField* field = parent->getField(i);
        jc::JsonNode* jvalue = nullptr;
        switch (field->type)
        {
        case mgpEarth::FeatureField::Int:
            jvalue = allocator->alloc_int(this->properties[field->index].intValue);
            break;
        case mgpEarth::FeatureField::Float:
            jvalue = allocator->alloc_float(this->properties[field->index].floatValue);
            break;
        case mgpEarth::FeatureField::String:
            jvalue = allocator->alloc_str(this->properties[field->index].strValue.c_str());
            break;
        default:
            jvalue = allocator->allocNode(jc::Type::Null);
            break;
        }
        jproperties->insert_pair(allocator->strdup(field->name.c_str()), jvalue);
    }
    jproperties->reverse();

    jc::JsonNode* geo = this->geometry.save(allocator);

    jc::JsonNode* jf = allocator->allocNode(jc::Type::Object);
    jf->insert_pair("properties", jproperties);
    jf->insert_pair("geometry", geo);
    jf->insert_pair("type", allocator->alloc_str("Feature"));
    return jf;
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

jc::JsonNode* Geometry::savePoint(int i, jc::JsonAllocator* allocator)
{
    jc::JsonNode* coord = allocator->allocNode(jc::Type::Array);
    int pos = i * 3;
    coord->insert(allocator->alloc_float(coordinates[pos + 2]));
    coord->insert(allocator->alloc_float(coordinates[pos + 1]));
    coord->insert(allocator->alloc_float(coordinates[pos]));
    return coord;
}

jc::JsonNode* Geometry::saveLine(int lineIdx, jc::JsonAllocator* allocator)
{
    jc::JsonNode* coord = allocator->allocNode(jc::Type::Array);
    for (int i = lines[lineIdx].startPoint; i < lines[lineIdx].size; ++i) {
        coord->insert(savePoint(i, allocator));
    }
    coord->reverse();
    return coord;
}

jc::JsonNode* Geometry::savePolygon(jc::JsonAllocator* allocator)
{
    jc::JsonNode* coord = allocator->allocNode(jc::Type::Array);
    for (int i = 0; i < lines.size(); ++i) {
        coord->insert(saveLine(i, allocator));
    }
    coord->reverse();
    return coord;
}

jc::JsonNode* Geometry::saveGeomertyCollection(jc::JsonAllocator* allocator)
{
    jc::JsonNode* jcoordinates = allocator->allocNode(jc::Type::Array);
    if (this->geometries.size() > 0) {
        for (int i = 0; i < geometries.size(); ++i) {
            jcoordinates->insert(geometries[i]->savePolygon(allocator));
        }
        jcoordinates->reverse();
    }
    return jcoordinates;
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

jc::JsonNode* Geometry::save(jc::JsonAllocator* allocator)
{
    jc::JsonNode* jgeo = allocator->allocNode(jc::Type::Object);

    jc::JsonNode* jcoordinates = NULL;
    const char* typeStr = "Unknow";
    switch (type)
    {
    case mgpEarth::GeometryType::Point:
        typeStr = "Point";
        jcoordinates = savePoint(0, allocator);
        break;
    case mgpEarth::GeometryType::MultiPoint:
        typeStr = "MultiPoint";
        jcoordinates = saveLine(0, allocator);
        break;
    case mgpEarth::GeometryType::LineString:
        typeStr = "LineString";
        jcoordinates = saveLine(0, allocator);
        break;
    case mgpEarth::GeometryType::MultiLineString:
        typeStr = "MultiLineString";
        jcoordinates = savePolygon(allocator);
        break;
    case mgpEarth::GeometryType::Polygon:
        typeStr = "Polygon";
        jcoordinates = savePolygon(allocator);
        break;
    case mgpEarth::GeometryType::MultiPolygon:
        typeStr = "MultiPolygon";
        jcoordinates = saveGeomertyCollection(allocator);
        break;
    case mgpEarth::GeometryType::GeometryCollection:
        typeStr = "GeometryCollection";
        jcoordinates = saveGeomertyCollection(allocator);
        break;
    default:
        jcoordinates = allocator->allocNode(jc::Type::Array);
        break;
    }

    jgeo->insert_pair("coordinates", jcoordinates);
    jgeo->insert_pair("type", allocator->alloc_str(typeStr));
    return jgeo;
}

void Geometry::getPoint(mgp::Vector3& point, int i)
{
    double* coord = coordinates.data() + i * 3;
    point.set(coord[0], coord[1], coord[2]);
}

int Geometry::getPointCount()
{
    return coordinates.size()/3;
}
