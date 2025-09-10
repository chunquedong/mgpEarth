/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */

#ifndef __EMSCRIPTEN__
    #define EMSCRIPTEN_KEEPALIVE 
#else
    #include <emscripten.h>
#endif

FE_BEGIN_NAMESPACE

extern "C" {

void EMSCRIPTEN_KEEPALIVE fe_showFps(EarthApp* self, bool show);

bool EMSCRIPTEN_KEEPALIVE fe_removeNode(EarthApp* self, const char* name);

void EMSCRIPTEN_KEEPALIVE fe_addTileLayer(EarthApp* self, const char* name, const char* uri, const char* elevationUri, char* options);

void EMSCRIPTEN_KEEPALIVE fe_addSkybox(EarthApp* self, int dark, float min, float max);

void EMSCRIPTEN_KEEPALIVE fe_addGeoLayer(EarthApp* self, const char* name, const char* uri, char* options);

void EMSCRIPTEN_KEEPALIVE fe_addBuildingLayer(EarthApp* self, const char* name, const char* uri, char* options);

void EMSCRIPTEN_KEEPALIVE fe_add3dtiles(EarthApp* self, const char* name, const char* uri, double lng, double lat, double height, int lighting, char* options);

void EMSCRIPTEN_KEEPALIVE fe_addGroundGltf(EarthApp* self, const char* name, const char* uri, double lng, double lat, double height, int lighting, char* options);

void EMSCRIPTEN_KEEPALIVE fe_addLight(EarthApp* self, const char* name, double lng, double lat, float r, float g, float b);

void EMSCRIPTEN_KEEPALIVE fe_setPosition(EarthApp* self, double lng, double lat, double zoom);

void EMSCRIPTEN_KEEPALIVE fe_moveTop(EarthApp* self, double lng, double lat, int time, double zoom);
void EMSCRIPTEN_KEEPALIVE fe_zoomTo(EarthApp* self, double zoom, uint64_t time);

void EMSCRIPTEN_KEEPALIVE fe_rotateTo(EarthApp* self, float rx, float rz, uint64_t time);

void EMSCRIPTEN_KEEPALIVE fe_addMultiModel(EarthApp* self, const char* name, const char* uri, int lighting, char* options);

int EMSCRIPTEN_KEEPALIVE fe_updateModelInstance(EarthApp* self, const char* name, int id, double lng, double lat, double height, char* options);

void EMSCRIPTEN_KEEPALIVE fe_removeModelInstance(EarthApp* self, const char* name, int id);

void EMSCRIPTEN_KEEPALIVE fe_addEmptyGeoLayer(EarthApp* self, const char* name, int geotype, char* options);

bool EMSCRIPTEN_KEEPALIVE fe_addGeoFeature(EarthApp* self, const char* name, int geotype, double* coords, int pointNum, char* attributes);

int EMSCRIPTEN_KEEPALIVE fe_removeGeoFeatureLike(EarthApp* self, const char* name, const char* fieldName, const char* value);
bool EMSCRIPTEN_KEEPALIVE fe_removeGeoFeatureAt(EarthApp* self, const char* name, int index);

float EMSCRIPTEN_KEEPALIVE fe_getLoadProgress(EarthApp* self, const char* name);

bool EMSCRIPTEN_KEEPALIVE fe_showLoadProgress(EarthApp* self, const char* name);

bool EMSCRIPTEN_KEEPALIVE fe_syncPick(EarthApp* self, const char* name, int x, int y, char* layerName, double* target, long* idOrIndex);

double* EMSCRIPTEN_KEEPALIVE fe_xyzToLnglat(EarthApp* self, double x, double y, double z, double* target);

double* EMSCRIPTEN_KEEPALIVE fe_lnglatToXyz(EarthApp* self, double lng, double lat, double height, double* target);

void EMSCRIPTEN_KEEPALIVE fe_clearHighlight(EarthApp* self);

}

FE_END_NAMESPACE