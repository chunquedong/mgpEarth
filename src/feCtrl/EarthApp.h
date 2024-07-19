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


class EarthApp : public Application, public PickerListener, public Refable {
    Node *earth;
    Node* atmosphere = NULL;
    Node* _skybox = NULL;

    EarthCtrl *earthCtrl;
    Gesture gesture;
    SPtr<ElevationManager> elevation;
    UPtr<Font> font;

    Form* _progressView = nullptr;
    std::function<void()> _checkProgress;
    WeakPtr<Node> _progressNode;
 public:
    EarthApp();
    ~EarthApp();
    void initialize() override;
    void update(float elapsedTime) override;
    bool mouseEvent(MotionEvent &event) override;
    void render(float elapsedTime) override;
    void finalize() override;
public:
    Node* add3dtiles(const char* name, const char* uri, const Coord2D& coord, double height, int lighting);
    XyzTileManager* addTileLayer(const char *name, const char* uri, const char* elevationUri);
    void addGeoNode(UPtr<GeoNode> node);
    void insertGeoNode(UPtr<GeoNode> node);
    void addSkybox(int dark, double minDis, double maxDis);
    bool removeNode(const char* name);
    bool showLoadProgress(Node* node);
private:
    void updateSkybox();
    void addAtmosphere();
    void drawLocationText();
public:
    EarthCtrl *getEarthCtrl() { return earthCtrl; }
    ElevationManager* getElevation() { return elevation.get(); }
    Node* getEarth() { return earth; }

    struct PickResult {
        std::string path;
        Node* layer;
        long userId;
        Drawable* drawable;
        int drawableIndex;
    };
    static bool getPickResult(RayQuery& result, PickResult& pickResult);
protected:
    virtual bool onPick(float x, float y, RayQuery& result) override;

    virtual bool onPickNode(PickResult& pickResult);
};

FE_END_NAMESPACE

#endif // EARTHAPP_H
