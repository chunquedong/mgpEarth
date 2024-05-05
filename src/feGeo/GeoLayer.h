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

class GeoLayer : public GeoNode {
    LabelSet* _label = nullptr;
    mgp::Line* _line = nullptr;
    mgp::Polygon* _polygon = nullptr;
public:
    UPtr<FeatureCollection> featureCollection;
    mgp::LineStyle lineStyle;
    mgp::PolyonStyle polygonStyle;
    mgp::LabelStyle labelStyle;
    std::string labelField;
    double height;
    double outlineHeightOffset;
    bool fillPolygon;
    bool strokePolygon;
    bool queryElevation;
    bool isLnglat = true;
    Vector3 baseTranslate;
public:
    GeoLayer(const char* uri);
    ~GeoLayer();

    virtual void* decodeFile(const char* path, NetResponse& lastRes, MultiRequest* req) override;
protected:
    Node* makeNode(FeatureCollection* fc);
public:
    void initEmpty(GeometryType geoType);
    void updateData();

    Drawable* getDrawable();
public:
    void write(Stream* file);
    bool read(Stream* file);
    bool readToNode(Stream* file, Node* node);
public:
    bool loadOptions(char* json_str);
private:
    void addPoint(Feature* feature, Geometry* geometry, LabelSet* label, BillboardSet* billboard, int index);
    void addLine(Geometry* geometry, GeoLine& gline, Line* line, double height);
    void addPolygon(Geometry* geometry, Polygon* polygon);
    bool addGeometry(Feature* feature, Geometry* geometry,
        LabelSet* label, BillboardSet* billboard, Line* line, Polygon* polygon);
    void coordToXyz(double x, double y, double z, Vector& xyz, double additionalHeight, bool doTranslate = true);

    UPtr<Drawable> readDrawable(Stream* file);
};

FE_END_NAMESPACE

#endif // _GEO_LAYER_H
