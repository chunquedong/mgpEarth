/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#include "feModel/PyramidGrid.h"
#include "feModel/Mercator.h"
#include "feModel/GeoCoordSys.h"

FE_USING_NAMESPACE

PyramidGrid::PyramidGrid(Envelope &env) :
  envelope(env) {
  width = env.width();
  height = env.height();
  baseX = env.minX();
  baseY = env.maxY();
  flipY = true;
}

PyramidGrid *PyramidGrid::getDefault() {
  static PyramidGrid *instance = NULL;
  if (instance == NULL) {
    Envelope env;
    env.init(-20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892);
    instance = new PyramidGrid(env);
  }
  return instance;
}

PyramidGrid* PyramidGrid::getBD() {
    static PyramidGrid* instance = NULL;
    if (instance == NULL) {
        Envelope env;
        env.init(-33554432, -33554432, 33554432, 33554432);
        instance = new PyramidGrid(env);
    }
    return instance;
}

void PyramidGrid::tileEnvelope(Tile &tile, Envelope &env) {
  int num = 1 << tile.z;
  double tileWidth = width / num;
  double tileHeight = height / num;
  double tminX = (tile.x * tileWidth)  + baseX;

  double tminY = tile.y * tileHeight;
  tminY = flipY ? (baseY - tminY - tileHeight) : baseY + tminY;

  env.set(tminX, tminY, tileWidth, tileHeight);
}

void PyramidGrid::tileEnvelopeBL(Tile &tile, Envelope &env) {
  tileEnvelope(tile, env);
  Coord2D p1 = env.minPoint();
  Coord2D p2 = env.maxPoint();
  GeoCoordSys::earth()->fromMercator(&p1);
  GeoCoordSys::earth()->fromMercator(&p2);

  env.init(p1.x, p1.y, p2.x, p2.y);
}

Tile PyramidGrid::getTileAt(double x, double y, int level) {
  int num = 1 << level;
  double tileWidth = width / num;
  double tileHeight = height / num;

  int tx = (x-baseX)/tileWidth;
  int ty;
  if (flipY) {
      ty = (baseY - y) / tileHeight;
  }
  else {
      ty = (y - baseY) / tileHeight;
  }
  Tile tile;
  tile.init(tx, ty, level);
  return tile;
}

void PyramidGrid::tileScreenSize(int &w, int &h) {
  w = 256;
  h = 256;
}
