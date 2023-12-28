/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#ifndef PYRAMIDGRID_H
#define PYRAMIDGRID_H

#include "feUtil/common.h"
#include "feModel/Envelope.h"
#include "feModel/Tile.h"
#include "mgp.h"

//CF_USING_NAMESPACE
PF_USING_NAMESPACE

FE_BEGIN_NAMESPACE

class PyramidGrid {
  double width;
  double height;
  double baseX;
  double baseY;
  bool flipY;
  Envelope envelope;
public:
  PyramidGrid(Envelope &env);

  static PyramidGrid *getDefault();

  /**
   * get the tile envelope
   */
  void tileEnvelope(Tile &tile, Envelope &env);


  void tileEnvelopeBL(Tile &tile, Envelope &env);

  void tileScreenSize(int &w, int &h);

  Tile getTileAt(double x, double y, int level);
};

FE_END_NAMESPACE
#endif // PYRAMIDGRID_H
