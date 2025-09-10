/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#ifndef TILE_H
#define TILE_H

#include "feUtil/common.h"
#include "feModel/Envelope.h"


FE_BEGIN_NAMESPACE

/**
 * 2D Rectangle range
 */
class Tile
{
public:
  int x;
  int y;
  int z;

  Tile();

  void init(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  /*
     2 | 3
     --+--
     0 | 1
  */
  void subTile(Tile subTiles[4]);

  Tile parent();

  bool operator< (const Tile &other) const {
    if (x < other.x) return true;
    if (x > other.x) return false;
    if (y < other.y) return true;
    if (y > other.y) return false;
    if (z < other.z) return true;
    if (z > other.z) return false;
    return false;
  }

  bool operator== (const Tile &other) const {
    return x == other.x
        && y == other.y
        && z == other.z;
  }

  size_t hashCode() const {
    size_t hashValue = x;
    hashValue = y + 31 * hashValue;
    hashValue = z + 31 * hashValue;
    return hashValue;
  }

  void toQuadKey(std::string& quadKey);
  void fromQuadKey(const std::string& quadKey);
};

FE_END_NAMESPACE

namespace std {
  template <> struct hash<mgpEarth::Tile> {
    size_t operator()(const mgpEarth::Tile &key) const {
      return key.hashCode();
    }
  };
}


#endif // TILE_H
