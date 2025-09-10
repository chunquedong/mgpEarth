/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#ifndef EARTHAPP_H
#define EARTHAPP_H

#include "mgp_pro.h"
#include "feCtrl/EarthCtrl.h"
#include "feGeo/Building.h"

FE_BEGIN_NAMESPACE


class EarthApp : public mgp::Application, public mgp::PickerListener, public mgp::Refable {
    mgp::Node *earth;
    mgp::Node* atmosphere = NULL;
    mgp::Node* _skybox = NULL;

    EarthCtrl *earthCtrl;
    mgp::Gesture gesture;
    mgp::SPtr<ElevationManager> elevation;
    mgp::UPtr<mgp::Font> font;

    mgp::Form* _progressView = nullptr;
    std::function<void()> _checkProgress;
    mgp::WeakPtr<mgp::Node> _progressNode;
 public:
    EarthApp();
    ~EarthApp();
    void initialize() override;
    void update(float elapsedTime) override;
    bool mouseEvent(mgp::MotionEvent &event) override;
    void render(float elapsedTime) override;
    void finalize() override;
public:
    mgp::Node* add3dtiles(const char* name, const char* uri, const Coord2D& coord, double height, int lighting);
    XyzTileManager* addTileLayer(const char *name, const char* uri, const char* elevationUri);
    void addGeoNode(mgp::UPtr<GeoNode> node);
    void insertGeoNode(mgp::UPtr<GeoNode> node);
    void addSkybox(int dark, double minDis, double maxDis);
    bool removeNode(const char* name);
    bool showLoadProgress(mgp::Node* node);
private:
    void updateSkybox();
    void addAtmosphere();
    void drawLocationText();
public:
    EarthCtrl *getEarthCtrl() { return earthCtrl; }
    ElevationManager* getElevation() { return elevation.get(); }
    mgp::Node* getEarth() { return earth; }

    struct PickResult {
        std::string path;
        mgp::Node* layer;
        long userId;
        mgp::Drawable* drawable;
        int drawableIndex;
    };
    static bool getPickResult(mgp::RayQuery& result, PickResult& pickResult);
protected:
    virtual bool onPick(float x, float y, mgp::RayQuery& result) override;

    virtual bool onPickNode(PickResult& pickResult);
};

FE_END_NAMESPACE

#endif // EARTHAPP_H
