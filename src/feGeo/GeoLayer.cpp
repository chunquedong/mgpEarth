/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#include "GeoLayer.h"
#include "feModel/GeoCoordSys.h"
#include "feElevation/Elevation.h"
#include "feCtrl/EarthCtrl.h"
#include "jparser.hpp"

using namespace jc;
FE_USING_NAMESPACE

GeoLayer::GeoLayer(const char* uri): GeoNode(uri), height(0), outlineHeightOffset(100),
        fillPolygon(true), strokePolygon(true), queryElevation(false) {
    labelStyle.sphereCulling = true;
    lineStyle.lineWidth = 1;
    lineStyle.lineColor = Vector4(0.0, 0.0, 1.0, 1.0);
    lineStyle.dashLen = 0;
    lineStyle.flowColor = Vector4(0.0, 1.0, 0.0, 1.0);
    lineStyle.isFlowing = false;
    //lineStyle.glowColor = Vector4(0.5, 1.0, 0.5, 1.0);
    polygonStyle.fillColor = Vector4(0.5, 0.7, 1.0, 1.0);
    labelField = "name";
    request->setSaveToFile(false);
}
GeoLayer::~GeoLayer() {
}

void* GeoLayer::decodeFile(const char* path, NetResponse& res, MultiRequest* req) {
    if (res.result.size() == 0) {
        return NULL;
    }

    if (StringUtil::endsWith(uri, ".geolayer")) {
        Buffer buffer((uint8_t*)res.result.data(), res.result.size(), false);

        UPtr<Node> node = Node::create();
        readToNode(&buffer, node.get());
        return node.take();
    }

    UPtr<FeatureCollection> fc(new FeatureCollection());
    if (!fc->parse(res.result)) {
        return NULL;
    }
    featureCollection = std::move(fc);
    return makeNode(featureCollection.get());
}

void GeoLayer::initEmpty(GeometryType geoType) {
    UPtr<FeatureCollection> fc(new FeatureCollection());
    fc->type = geoType;
    featureCollection = std::move(fc);

    this->removeAllChildren();

    UPtr<LabelSet> label = LabelSet::create(labelStyle);
    UPtr<mgp::Line> line = Line::create(lineStyle);
    UPtr<mgp::Polygon> polygon = Polygon::create(polygonStyle);

    this->addChild(Node::createForComponent(std::move(label)));
    this->addChild(Node::createForComponent(std::move(line)));
    this->addChild(Node::createForComponent(std::move(polygon)));
}

void GeoLayer::updateData() {
    LabelSet* label = nullptr;
    mgp::Line* line = nullptr;
    mgp::Polygon* polygon = nullptr;

    std::vector<Drawable*> list;
    this->getAllDrawable(list);
    for (int i = 0; i < list.size(); ++i) {
        Drawable* drawable = list[i];
        if (LabelSet* label_ = dynamic_cast<LabelSet*>(drawable)) {
            label = label_;
        }
        else if (Line* line_ = dynamic_cast<Line*>(drawable)) {
            line = line_;
        }
        if (Polygon* polygon_ = dynamic_cast<Polygon*>(drawable)) {
            polygon = polygon_;
        }
    }

    if (label) {
        label->clear();
    }
    if (line) {
        line->start();
    }
    if (polygon) {
        polygon->start();
    }

    FeatureCollection* fc = featureCollection.get();
    for (auto it = fc->features.begin(); it != fc->features.end(); ++it) {
        Feature* feature = *it;
        addGeometry(feature, &feature->geometry, label, NULL, line, polygon);
    }

    if (line) {
        line->finish();
    }
    if (polygon) {
        polygon->finish();
    }
}

Drawable* GeoLayer::getDrawable()
{
    LabelSet* label = nullptr;
    mgp::Line* line = nullptr;
    mgp::Polygon* polygon = nullptr;

    std::vector<Drawable*> list;
    this->getAllDrawable(list);
    for (int i = 0; i < list.size(); ++i) {
        Drawable* drawable = list[i];
        if (LabelSet* label_ = dynamic_cast<LabelSet*>(drawable)) {
            label = label_;
        }
        else if (Line* line_ = dynamic_cast<Line*>(drawable)) {
            line = line_;
        }
        if (Polygon* polygon_ = dynamic_cast<Polygon*>(drawable)) {
            polygon = polygon_;
        }
    }

    if (featureCollection->type == GeometryType::Point) {
        return label;
    }
    else if (featureCollection->type == GeometryType::LineString) {
        return line;
    }
    else if (featureCollection->type == GeometryType::Polygon) {
        return polygon;
    }

    if (polygon && polygon->getBatchSize()) {
        polygon;
    }
    if (line && line->getBatchSize()) {
        line;
    }
    if (label && label->size()) {
        return label;
    }
    return nullptr;
}

Node* GeoLayer::makeNode(FeatureCollection* fc) {
    if (!fc) {
        return NULL;
    }

    UPtr<LabelSet> label = LabelSet::create(labelStyle);
    UPtr<mgp::Line> line = Line::create(lineStyle);
    UPtr<mgp::Polygon> polygon = Polygon::create(polygonStyle);

    line->start();
    polygon->start();

    for (auto it = fc->features.begin(); it != fc->features.end(); ++it) {
        Feature* feature = *it;
        addGeometry(feature, &feature->geometry, label.get(), NULL, line.get(), polygon.get());
    }

    line->finish();
    polygon->finish();

    UPtr<Node> node = Node::create();
    node->addChild(Node::createForComponent(std::move(label)));
    node->addChild(Node::createForComponent(std::move(line)));
    node->addChild(Node::createForComponent(std::move(polygon)));
    return node.take();
}

//void GeoLayer::onReceive(Task* task, NetResponse &res) {
//    if (res.decodeResult) {
//        this->addChild(UPtr<Node>((Node*)res.decodeResult));
//    }
//}

bool GeoLayer::addGeometry(Feature* feature, Geometry* geometry, 
    LabelSet* label, BillboardSet* billboard, Line* line, Polygon* polygon) {
    switch (geometry->type) {
    case GeometryType::Point:
        addPoint(feature, geometry, label, billboard, 0);
        break;
    case GeometryType::LineString:
        addLine(geometry, geometry->lines.at(0), line, height);
        break;
    case GeometryType::Polygon:
        if (fillPolygon) {
            addPolygon(geometry, polygon);
        }
        if (strokePolygon && geometry->lines.size() > 0) {
            addLine(geometry, geometry->lines.at(0), line, height+outlineHeightOffset);
            line->setPickMask(0);
        }
        break;
    case GeometryType::MultiPoint:
        if (geometry->lines.size() > 0) {
            for (int i = 0; i < geometry->lines[0].size; ++i) {
                addPoint(feature, geometry, label, billboard, i+geometry->lines[0].startPoint);
            }
        }
        break;
    case GeometryType::MultiLineString:
        for (GeoLine& g : geometry->lines) {
            addLine(geometry, g, line, height + outlineHeightOffset);
        }
        break;
    case GeometryType::MultiPolygon:
    case GeometryType::GeometryCollection:
        for (Geometry* g : geometry->geometries) {
            addGeometry(feature, g, label, billboard, line, polygon);
        }
        break;
    default:
        print("unsupport geometry type:%d\n", geometry->type);
        return false;
    }
    return true;
}

void GeoLayer::coordToXyz(double x, double y, double z, Vector& point, double height) {
    double radius = GeoCoordSys::earth()->getRadius() + height;
    if (z == 0 && queryElevation) {
        z = OfflineElevation::cur()->getHeight(x, y, 20);
    }
    radius += z;

    Coord2D coord(x, y);
    GeoCoordSys::blToXyz(coord, point, radius);
}


void GeoLayer::addLine(Geometry* geometry, GeoLine& gline, Line* line, double height) {
    std::vector<float> coords;
    Vector point;
    //Vector3 offset;
    for (int i = gline.startPoint; i < gline.startPoint+gline.size; ++i) {
        int pos = i * 3;
        double x = geometry->coordinates[pos];
        double y = geometry->coordinates[pos+1];
        double z = geometry->coordinates[pos+2];

        coordToXyz(x, y, z, point, height);

        coords.push_back(point.x);
        coords.push_back(point.y);
        coords.push_back(point.z);
    }

    line->add(coords.data(), coords.size()/3);
}

void GeoLayer::addPolygon(Geometry* geometry, Polygon* polygon) {
    std::vector<uint32_t> indices;
    std::vector<float> coords = geometry->coordinates;

    mgp::Polygon::triangulate(coords.data(), 3, (int*)geometry->lines.data(),
        geometry->lines.size(), indices);
    Vector point;
    for (int i = 0; i < coords.size(); i += 3) {
        double x = coords[i];
        double y = coords[i + 1];
        double z = coords[i + 2];
        
        coordToXyz(x, y, z, point, height);

        coords[i] = point.x;
        coords[i + 1] = point.y;
        coords[i + 2] = point.z;
    }

    polygon->add(coords.data(), coords.size() / 3, indices.data(), indices.size());
}

void GeoLayer::addPoint(Feature* feature, Geometry* geometry, LabelSet* label, BillboardSet* billboard, int index) {
    double x = geometry->coordinates[0 + 3*index];
    double y = geometry->coordinates[1 + 3 * index];
    double z = geometry->coordinates[2 + 3 * index];
    Vector point;
    coordToXyz(x, y, z, point, height);
    //Vector point(x, y, z);
    if (feature) {
        if (labelField.size() > 0) {
            auto it = feature->properties.find(labelField);
            if (it != feature->properties.end()) {
                label->add(point, it->second);
            }
        }
        else {
            label->add(point, "");
        }
    }
}

static void writeDrawable(Drawable* drawable, Stream* file) {
    LabelSet* label = dynamic_cast<LabelSet*>(drawable);
    if (label) {
        file->writeInt32(1);
        label->write(file);
        return;
    }
    Line* line = dynamic_cast<Line*>(drawable);
    if (line) {
        file->writeInt32(2);
        line->write(file);
        return;
    }
    Polygon* polygon = dynamic_cast<Polygon*>(drawable);
    if (polygon) {
        file->writeInt32(3);
        polygon->write(file);
        return;
    }
}

void GeoLayer::write(Stream* file) {
    std::vector<Drawable*> drawables;
    this->getAllDrawable(drawables);
    file->writeUInt32(drawables.size());
    for (Drawable* d : drawables) {
        writeDrawable(d, file);
    }
}

UPtr<Drawable> GeoLayer::readDrawable(Stream* file) {
    int type = file->readInt32();
    switch (type) {
    case 1: {
        UPtr<LabelSet> label = LabelSet::create(labelStyle);
        label->read(file);
        return label;
    }
    case 2: {
        UPtr<mgp::Line> line = Line::create(lineStyle);
        line->start();
        line->read(file);
        return line;
    }
    case 3: {
        UPtr<mgp::Polygon> polygon = Polygon::create(polygonStyle);
        polygon->start();
        polygon->read(file);
        return polygon;
    }
    }
    return UPtr<Drawable>();
}

bool GeoLayer::read(Stream* file) {
    return readToNode(file, this);
}

bool GeoLayer::readToNode(Stream* file, Node* node) {
    int size = file->readUInt32();
    for (int i = 0; i < size; ++i) {
        auto drawable = readDrawable(file);
        node->addChild(Node::createForComponent(std::move(drawable)));
    }
    return true;
}

static void parserColor(Value* json, const char* name, Vector4& color) {
    Value* lineColor = json->get(name);
    if (lineColor && lineColor->size() == 4) {
        auto c = lineColor->begin();
        color.x = c->as_float();
        ++c;
        color.y = c->as_float();
        ++c;
        color.z = c->as_float();
        ++c;
        color.w = c->as_float();
        //++c;
    }
}

bool GeoLayer::loadOptions(char* json_str) {
    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* value0 = parser.parse(json_str);

    if (!value0 || parser.get_error()[0] != 0) {
        printf("parser options json error: %s\n", parser.get_error());
        printf("%s\n", json_str);
        return false;
    }

    GeoLayer* road = this;

    Value* max = value0->get("maxDis");
    if (max) {
        road->maxDis = max->as_float();
    }

    Value* min = value0->get("minDis");
    if (min) {
        road->minDis = min->as_float();
    }

    Value* height = value0->get("height");
    if (height) {
        road->height = height->as_float();
    }

    Value* labelField = value0->get("labelField");
    if (labelField) {
        road->labelField = labelField->as_str();
    }

    Value* outlineHeightOffset = value0->get("outlineHeightOffset");
    if (outlineHeightOffset) {
        road->outlineHeightOffset = outlineHeightOffset->as_float();
    }

    Value* fillPolygon = value0->get("fillPolygon");
    if (fillPolygon) {
        road->fillPolygon = fillPolygon->as_bool();
    }

    Value* strokePolygon = value0->get("strokePolygon");
    if (strokePolygon) {
        road->strokePolygon = strokePolygon->as_bool();
    }

    Value* queryElevation = value0->get("queryElevation");
    if (queryElevation) {
        road->queryElevation = queryElevation->as_bool();
    }

    Value* lineStyle = value0->get("lineStyle");
    if (lineStyle) {
        Value* depthTest = lineStyle->get("depthTest");
        if (depthTest) {
            road->lineStyle.depthTest = depthTest->as_bool();
        }

        Value* lineWidth = lineStyle->get("lineWidth");
        if (lineWidth) {
            road->lineStyle.lineWidth = lineWidth->as_float();
        }

        Value* dashLen = lineStyle->get("dashLen");
        if (dashLen) {
            road->lineStyle.dashLen = dashLen->as_float();
        }

        Value* isFlowing = lineStyle->get("isFlowing");
        if (isFlowing) {
            road->lineStyle.isFlowing = isFlowing->as_bool();
        }

        Value* hasGlow = lineStyle->get("hasGlow");
        if (hasGlow) {
            road->lineStyle.hasGlow = hasGlow->as_bool();
        }

        parserColor(lineStyle, "lineColor", road->lineStyle.lineColor);
        parserColor(lineStyle, "flowColor", road->lineStyle.flowColor);
    }
    
    Value* polygonStyle = value0->get("polygonStyle");
    if (polygonStyle) {
        Value* depthTest = polygonStyle->get("depthTest");
        if (depthTest) {
            road->polygonStyle.depthTest = depthTest->as_bool();
        }
        parserColor(polygonStyle, "fillColor", road->polygonStyle.fillColor);
    }

    Value* labelStyle = value0->get("labelStyle");
    if (labelStyle) {
        Value* iconSize = labelStyle->get("iconSize");
        if (iconSize) {
            road->labelStyle.iconSize = iconSize->as_float();
        }
        Value* fontSize = labelStyle->get("fontSize");
        if (fontSize) {
            road->labelStyle.fontSize = fontSize->as_int();
        }
        Value* iconImage = labelStyle->get("iconImage");
        if (iconImage) {
            road->labelStyle.iconImage = iconImage->as_str();
        }
        Value* fontName = labelStyle->get("fontName");
        if (fontName) {
            road->labelStyle.fontName = fontName->as_str();
        }
        Value* iconRect = labelStyle->get("iconRect");
        if (iconRect) {
            if (iconRect && iconRect->size() == 4) {
                auto c = iconRect->begin();
                road->labelStyle.iconRect.x = c->as_float();
                ++c;
                road->labelStyle.iconRect.y = c->as_float();
                ++c;
                road->labelStyle.iconRect.width = c->as_float();
                ++c;
                road->labelStyle.iconRect.height = c->as_float();
                //++c;
            }
        }
        Value* labelAlign = labelStyle->get("labelAlign");
        if (labelAlign) {
            road->labelStyle.labelAlign = (LabelStyle::LabelAlign)labelAlign->as_int();
        }
        Value* sphereCulling = labelStyle->get("sphereCulling");
        if (sphereCulling) {
            road->labelStyle.sphereCulling = sphereCulling->as_bool();
        }

        Value* coverStrategy = labelStyle->get("coverStrategy");
        if (coverStrategy) {
            road->labelStyle.coverStrategy = coverStrategy->as_int();
        }

        Value* textOffsetX = labelStyle->get("textOffsetX");
        if (textOffsetX) {
            road->labelStyle.textOffsetX = textOffsetX->as_float();
        }

        Value* textOffsetY = labelStyle->get("textOffsetY");
        if (textOffsetY) {
            road->labelStyle.textOffsetY = textOffsetY->as_float();
        }

        parserColor(labelStyle, "fontColor", road->labelStyle.fontColor);
        parserColor(labelStyle, "iconColor", road->labelStyle.iconColor);
    }
    return true;
}