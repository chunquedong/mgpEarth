/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef _GEO_LAYER_H
#define _GEO_LAYER_H

#include "feUtil/common.h"
#include "mgp_pro.h"
#include "feModel/Envelope.h"
#include "Geometry.h"
#include "feElevation/Elevation.h"
#include "feObjects/GeoNode.h"
#include <float.h>

FE_BEGIN_NAMESPACE
PF_USING_NAMESPACE

class GeoLayer;

class Symbolizer {
    friend class GeoLayer;
public:
    LabelSet* _label = nullptr;
    mgp::Line* _line = nullptr;
    mgp::Polygon* _polygon = nullptr;
    GeoLayer* _layer = nullptr;

public:
    double minDis = 0;
    double maxDis = FLT_MAX;

    struct Filter {
        std::string fieldName;
        std::string op = "==";
        FeatureValue value;
        FeatureField::Type valueType;

        int64_t getIntValue();
        double getFloatValue();
    };
    std::vector<Filter> filters;

    mgp::LineStyle lineStyle;
    mgp::PolyonStyle polygonStyle;
    mgp::LabelStyle labelStyle;
    std::string labelField;
    bool fillPolygon = true;
    bool strokePolygon = true;
    bool pickable = true;
    double outlineHeightOffset = 100;
    int curveType = 0;

    Symbolizer();

    void initAddTo(Node* node);

    void beginUpdate();
    void endUpdate();
    bool updateRender(Feature* feature, int id);
private:
    void addPoint(Feature* feature, Geometry* geometry, int index, int id);
    void addLine(Geometry* geometry, GeoLine& gline, bool isOutline, int id);
    void addPolygon(Geometry* geometry, int id);
    bool addGeometry(Feature* feature, Geometry* geometry, int id);
public:
    bool loadOptions(jc::Value* json);
};

class GeoLayer : public GeoNode {
    bool _featuresDirty = false;
public:
    UPtr<FeatureCollection> featureCollection;

    double additionalHeight = 0;
    bool queryElevation = false;
    bool isLnglat = true;
    bool polygonInterpolation = false;
    Vector3 baseTranslate;

    std::vector<Symbolizer> symbolizers;

    Symbolizer* getSymb() { return &symbolizers[0]; }
public:
    GeoLayer(const char* uri);
    ~GeoLayer();

    virtual void* decodeFile(const char* path, NetResponse& lastRes, MultiRequest* req) override;
protected:
    Node* makeNode(FeatureCollection* fc);
public:
    void initEmpty(GeometryType geoType);
    void updateData();
    void doUpdateRenderData();

    //custom lable color
    virtual Vector4* getColor(int i);
protected:
    void update(float elapsedTime) override;
    void onReceive(Task* task, NetResponse& res, MultiRequest* req) override;

public:
    bool loadOptions(char* json_str);
public:
    void coordToXyz(double x, double y, double z, Vector& xyz, double additionalHeight, bool doTranslate = true);
};

FE_END_NAMESPACE

#endif // _GEO_LAYER_H
