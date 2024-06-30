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

Symbolizer::Symbolizer()
{
    labelStyle.sphereCulling = true;
    lineStyle.lineWidth = 1;
    lineStyle.lineColor = Vector4(0.0, 0.0, 1.0, 1.0);
    lineStyle.dashLen = 0;
    lineStyle.flowColor = Vector4(0.0, 1.0, 0.0, 1.0);
    lineStyle.flowSpeed = 0;
    //lineStyle.glowColor = Vector4(0.5, 1.0, 0.5, 1.0);
    polygonStyle.fillColor = Vector4(0.5, 0.7, 1.0, 1.0);
    labelField = "name";
}

void Symbolizer::initAddTo(Node* node) {
    UPtr<LabelSet> label = LabelSet::create(labelStyle);
    UPtr<mgp::Line> line = Line::create(lineStyle);
    UPtr<mgp::Polygon> polygon = Polygon::create(polygonStyle);

    _label = label.get();
    _line = line.get();
    _polygon = polygon.get();

    if (!pickable) {
        _label->setPickMask(0);
        _line->setPickMask(0);
        _polygon->setPickMask(0);
    }

    //UPtr<Node> node = Node::create();
    node->addChild(Node::createForComponent(std::move(label)));
    node->addChild(Node::createForComponent(std::move(line)));
    node->addChild(Node::createForComponent(std::move(polygon)));
}

void Symbolizer::beginUpdate() {
    if (!_label) {
        return;
    }

    if (_label) {
        _label->clear();
    }
    if (_line) {
        _line->start();
    }
    if (_polygon) {
        _polygon->start();
    }
}
void Symbolizer::endUpdate() {
    if (!_label) {
        return;
    }

    if (_line) {
        _line->finish();
    }
    if (_polygon) {
        _polygon->finish();
    }

    if (!_layer->baseTranslate.isZero()) {
        if (_line) _line->getNode()->setTranslation(_layer->baseTranslate);
        if (_polygon) _polygon->getNode()->setTranslation(_layer->baseTranslate);
    }
}


int64_t Symbolizer::Filter::getIntValue()
{
    if (valueType == FeatureField::Type::Int)
        return value.intValue;
    else if (valueType == FeatureField::Type::Float)
        return value.floatValue;
    return -1;
}


double Symbolizer::Filter::getFloatValue()
{
    if (valueType == FeatureField::Type::Int)
        return value.intValue;
    else if (valueType == FeatureField::Type::Float)
        return value.floatValue;
    return -1;
}


bool Symbolizer::updateRender(Feature* feature, int id) {
    if (!_label) {
        return false;
    }

    for (Filter& filter : filters) {
        FeatureField* field = feature->parent->getField(filter.fieldName);
        if (!field)
            return false;
        int fieldIndex = field->index;
        bool res = true;
        switch (field->type)
        {
        case FeatureField::Bool:
        case FeatureField::Int:
            if (filter.op == "==") {
                res = feature->getInt(fieldIndex) == filter.getIntValue();
            }
            else if (filter.op == "!=") {
                res = feature->getInt(fieldIndex) != filter.getIntValue();
            }
            else if (filter.op == ">") {
                res = feature->getInt(fieldIndex) > filter.getIntValue();
            }
            else if (filter.op == "<") {
                res = feature->getInt(fieldIndex) < filter.getIntValue();
            }
            else if (filter.op == ">=") {
                res = feature->getInt(fieldIndex) >= filter.getIntValue();
            }
            else if (filter.op == "<=") {
                res = feature->getInt(fieldIndex) <= filter.getIntValue();
            }
            break;
        case FeatureField::Float:
            if (filter.op == "==") {
                res = feature->getFloat(fieldIndex) == filter.getFloatValue();
            }
            else if (filter.op == "!=") {
                res = feature->getFloat(fieldIndex) != filter.getFloatValue();
            }
            else if (filter.op == ">") {
                res = feature->getFloat(fieldIndex) > filter.getFloatValue();
            }
            else if (filter.op == "<") {
                res = feature->getFloat(fieldIndex) < filter.getFloatValue();
            }
            else if (filter.op == ">=") {
                res = feature->getFloat(fieldIndex) >= filter.getFloatValue();
            }
            else if (filter.op == "<=") {
                res = feature->getFloat(fieldIndex) <= filter.getFloatValue();
            }
            break;
        case FeatureField::String:
            if (filter.op == "==") {
                res = feature->getStr(fieldIndex) == filter.value.strValue;
            }
            else if (filter.op == "!=") {
                res = feature->getStr(fieldIndex) != filter.value.strValue;
            }
            else if (filter.op == ">") {
                res = feature->getStr(fieldIndex) > filter.value.strValue;
            }
            else if (filter.op == "<") {
                res = feature->getStr(fieldIndex) < filter.value.strValue;
            }
            else if (filter.op == ">=") {
                res = feature->getStr(fieldIndex) >= filter.value.strValue;
            }
            else if (filter.op == "<=") {
                res = feature->getStr(fieldIndex) <= filter.value.strValue;
            }
            break;
        default:
            break;
        }
        if (!res)
            return false;
    }
    addGeometry(feature, &feature->geometry, id);
    return true;
}

bool Symbolizer::addGeometry(Feature* feature, Geometry* geometry, int id) {
    switch (geometry->type) {
    case GeometryType::Point:
        addPoint(feature, geometry, 0, id);
        break;
    case GeometryType::LineString:
        addLine(geometry, geometry->lines.at(0), false, id);
        break;
    case GeometryType::Polygon:
        if (fillPolygon) {
            addPolygon(geometry, id);
        }
        if (strokePolygon && geometry->lines.size() > 0) {
            addLine(geometry, geometry->lines.at(0), true, id);
            _line->setPickMask(0);
        }
        break;
    case GeometryType::MultiPoint:
        if (geometry->lines.size() > 0) {
            for (int i = 0; i < geometry->lines[0].size; ++i) {
                addPoint(feature, geometry, i + geometry->lines[0].startPoint, id);
            }
        }
        break;
    case GeometryType::MultiLineString:
        for (GeoLine& g : geometry->lines) {
            addLine(geometry, g, false, id);
        }
        break;
    case GeometryType::MultiPolygon:
    case GeometryType::GeometryCollection:
        for (Geometry* g : geometry->geometries) {
            addGeometry(feature, g, id);
        }
        break;
    default:
        print("unsupport geometry type:%d\n", geometry->type);
        return false;
    }
    return true;
}


void Symbolizer::addLine(Geometry* geometry, GeoLine& gline, bool isOutline, int id) {
    double height = _layer->additionalHeight;
    if (isOutline) {
        height += outlineHeightOffset;
    }
    std::vector<float> coords;
    Vector point;
    //Vector3 offset;
    for (int i = gline.startPoint; i < gline.startPoint + gline.size; ++i) {
        int pos = i * 3;
        double x = geometry->coordinates[pos];
        double y = geometry->coordinates[pos + 1];
        double z = geometry->coordinates[pos + 2];

        _layer->coordToXyz(x, y, z, point, height);

        coords.push_back(point.x);
        coords.push_back(point.y);
        coords.push_back(point.z);
    }

    _line->add(coords.data(), coords.size() / 3, id);
}

void Symbolizer::addPolygon(Geometry* geometry, int id) {
    
    std::vector<double> coords = geometry->coordinates;

    if (_layer->polygonInterpolation) {
        mgp::Polygon::TriangeData out;
        mgp::Polygon::clipAndTriangulate(coords.data(), 3, (int*)geometry->lines.data(),
            geometry->lines.size(), 0.5, out);

        double avgZ = 0;
        for (int i = 0; i < coords.size(); i += 3) {
            double z = coords[i + 2];
            avgZ += z;
        }
        avgZ /= (coords.size()/3);

        std::vector<float> fcoords(out.coords.size()/2*3);
        Vector point;
        for (int i = 0; i < out.coords.size()/2; ++i) {
            int pos0 = i * 2;
            double x = out.coords[pos0];
            double y = out.coords[pos0 + 1];
            double z = avgZ;

            _layer->coordToXyz(x, y, z, point, _layer->additionalHeight);

            int pos = i * 3;
            fcoords[pos] = point.x;
            fcoords[pos + 1] = point.y;
            fcoords[pos + 2] = point.z;
        }

        _polygon->add(fcoords.data(), fcoords.size() / 3, out.indexBuf.data(), out.indexBuf.size(), id);
    }
    else {
        std::vector<uint32_t> indices;
        mgp::Polygon::triangulate(coords.data(), 3, (int*)geometry->lines.data(),
            geometry->lines.size(), indices);

        std::vector<float> fcoords(coords.size());
        Vector point;
        for (int i = 0; i < coords.size(); i += 3) {
            double x = coords[i];
            double y = coords[i + 1];
            double z = coords[i + 2];

            _layer->coordToXyz(x, y, z, point, _layer->additionalHeight);

            fcoords[i] = point.x;
            fcoords[i + 1] = point.y;
            fcoords[i + 2] = point.z;
        }

        _polygon->add(fcoords.data(), fcoords.size() / 3, indices.data(), indices.size(), id);
    }
}

void Symbolizer::addPoint(Feature* feature, Geometry* geometry, int index, int id) {
    double x = geometry->coordinates[0 + 3 * index];
    double y = geometry->coordinates[1 + 3 * index];
    double z = geometry->coordinates[2 + 3 * index];
    Vector point;
    _layer->coordToXyz(x, y, z, point, _layer->additionalHeight, false);
    //Vector point(x, y, z);
    if (feature) {
        if (labelField.size() > 0) {
            std::string value;
            feature->getAsStr(labelField, value);
            _label->add(point, value, id);
        }
        else {
            _label->add(point, "", id);
        }
    }
}

bool Symbolizer::loadOptions(jc::Value* value0)
{
    Symbolizer* road = this;

    Value* max = value0->get("maxDis");
    if (max) {
        road->maxDis = max->as_float();
    }

    Value* min = value0->get("minDis");
    if (min) {
        road->minDis = min->as_float();
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
        Value* dashGap = lineStyle->get("dashGap");
        if (dashGap) {
            road->lineStyle.dashGap = dashGap->as_bool();
        }
        Value* hasDashGapColor = lineStyle->get("hasDashGapColor");
        if (hasDashGapColor) {
            road->lineStyle.hasDashGapColor = hasDashGapColor->as_bool();
        }

        Value* dashFlowSpeed = lineStyle->get("dashFlowSpeed");
        if (dashFlowSpeed) {
            road->lineStyle.dashFlowSpeed = dashFlowSpeed->as_float();
        }

        Value* flowSpeed = lineStyle->get("flowSpeed");
        if (flowSpeed) {
            road->lineStyle.flowSpeed = flowSpeed->as_float();
        }

        Value* arrowSize = lineStyle->get("arrowSize");
        if (arrowSize) {
            road->lineStyle.arrowSize = arrowSize->as_float();
        }

        Value* arrowWithScale = lineStyle->get("arrowWithScale");
        if (arrowWithScale) {
            road->lineStyle.arrowWithScale = arrowWithScale->as_float();
        }

        Value* worldSize = lineStyle->get("worldSize");
        if (worldSize) {
            road->lineStyle.worldSize = worldSize->as_bool();
        }

        Value* glowPower = lineStyle->get("glowPower");
        if (glowPower) {
            road->lineStyle.glowPower = glowPower->as_float();
        }

        parserColor(lineStyle, "lineColor", road->lineStyle.lineColor);
        parserColor(lineStyle, "flowColor", road->lineStyle.flowColor);
        parserColor(lineStyle, "dashGapColor", road->lineStyle.dashGapColor);
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

    Value* jfilters = value0->get("filters");
    if (jfilters) {
        for (auto it = jfilters->begin(); it != jfilters->end(); ++it) {
            Filter filter;
            filter.fieldName = it->get_str("fieldName", "");
            filter.op = it->get_str("op", "==");
            Value* jf = it->get("value");
            if (!jf)
                continue;
            switch (jf->type())
            {
            case jc::Type::Integer:
                filter.value.intValue = jf->as_int();
                filter.valueType = FeatureField::Type::Int;
                break;
            case jc::Type::Float:
                filter.value.floatValue = jf->as_float();
                filter.valueType = FeatureField::Type::Float;
                break;
            case jc::Type::Boolean:
                filter.value.intValue = jf->as_bool();
                filter.valueType = FeatureField::Type::Bool;
                break;
            default:
                filter.value.strValue = jf->as_str();
                filter.valueType = FeatureField::Type::String;
                break;
            }
            this->filters.push_back(filter);
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

GeoLayer::GeoLayer(const char* uri) : GeoNode(uri) {
    request->setSaveToFile(false);
    Symbolizer symb;
    symbolizers.push_back(symb);
}

GeoLayer::~GeoLayer() {
}

void* GeoLayer::decodeFile(const char* path, NetResponse& res, MultiRequest* req) {
    if (res.result.size() == 0) {
        return NULL;
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
    
    for (Symbolizer& sym : symbolizers) {
        sym._layer = this;
        sym.initAddTo(this);
    }
}

void GeoLayer::updateData()
{
    _featuresDirty = true;
}

void GeoLayer::update(float elapsedTime)
{
    GeoNode::update(elapsedTime);
    if (_featuresDirty) {
        doUpdateRenderData();
    }
}

void GeoLayer::doUpdateRenderData() {
    _featuresDirty = false;

    for (Symbolizer& sym : symbolizers) {
        sym.beginUpdate();
    }
    int i = 0;
    for (auto& feature : featureCollection->features) {
        for (Symbolizer& sym : symbolizers) {
            bool ok = true;
            if (ctrl) {
                double dis = ctrl->getDistanceToSurface();
                if (dis > sym.minDis && dis <= sym.maxDis) {
                }
                else {
                    ok = false;
                }
            }
            if (ok) {
                if (sym.updateRender(feature.get(), i)) {
                    goto outlabel;
                }
            }
        }
    outlabel:
        ++i;
        continue;
    }

    for (Symbolizer& sym : symbolizers) {
        sym.endUpdate();
    }
}

Node* GeoLayer::makeNode(FeatureCollection* fc) {
    if (!fc) {
        return NULL;
    }

    UPtr<Node> node = Node::create();
    for (Symbolizer& sym : symbolizers) {
        sym._layer = this;
        sym.initAddTo(node.get());
    }
    
    doUpdateRenderData();

    return node.take();
}

void GeoLayer::onReceive(Task* task, NetResponse& res, MultiRequest* req)
{
    if (res.decodeResult) {
        this->removeAllChildren();
    }
    GeoNode::onReceive(task, res, req);
}

void GeoLayer::coordToXyz(double x, double y, double z, Vector& point, double additionalHeight, bool doTranslate) {

    if (!isLnglat) {
        point.set(x, y, z);
        if (additionalHeight) {
            Vector3 offset;
            point.normalize(&offset);
            point += offset * additionalHeight;
        }
        if (doTranslate) {
            if (baseTranslate.isZero()) {
                baseTranslate = point;
                point.set(0, 0, 0);
            }
            else {
                point = point - baseTranslate;
            }
        }
        return;
    }

    double radius = GeoCoordSys::earth()->getRadius() + additionalHeight;
    if (z == 0 && queryElevation) {
        z = OfflineElevation::cur()->getHeight(x, y, 20);
    }
    radius += z;

    Coord2D coord(x, y);
    GeoCoordSys::blToXyz(coord, point, radius);

    if (doTranslate) {
        if (baseTranslate.isZero()) {
            baseTranslate = point;
            point.set(0, 0, 0);
        }
        else {
            point = point - baseTranslate;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool GeoLayer::loadOptions(char* json_str) {
    //printf("json: %s\n", json_str);
    
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

    Value* additionalHeight = value0->get("additionalHeight");
    if (additionalHeight) {
        road->additionalHeight = additionalHeight->as_float();
    }

    Value* polygonInterpolation = value0->get("polygonInterpolation");
    if (polygonInterpolation) {
        road->polygonInterpolation = polygonInterpolation->as_bool();
    }

    Value* queryElevation = value0->get("queryElevation");
    if (queryElevation) {
        road->queryElevation = queryElevation->as_bool();
    }

    Value* jsymbolizers = value0->get("symbolizers");
    if (jsymbolizers) {
        symbolizers.resize(jsymbolizers->size());
        int i = 0;
        for (auto it = jsymbolizers->begin(); it != jsymbolizers->end(); ++it) {
            Value* jsym = *it;
            symbolizers[i].loadOptions(jsym);
            //symbolizers[i]._layer = this;
            ++i;
        }
    }

    return true;
}
