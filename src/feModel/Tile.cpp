/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#include "feModel/Tile.h"

FE_USING_NAMESPACE

Tile::Tile(): x(0), y(0), z(0) {
}

/*
   2 | 3
   --+--
   0 | 1
*/
void Tile::subTile(Tile subTiles[4]) {
  int x = this->x << 1;
  int y = this->y << 1;
  int z = this->z + 1;

  subTiles[0].init(x, y, z);
  subTiles[1].init(x+1, y, z);
  subTiles[2].init(x, y+1, z);
  subTiles[3].init(x+1, y+1, z);
}


Tile Tile::parent() {
  if (this->z == 0) {
    return *this;
  }
  
  int x = this->x/2;
  int y = this->y/2;
  int z = this->z-1;
  Tile p;
  p.init(x, y, z);
  return p;
}

void Tile::toQuadKey(std::string &quadKey)
{
    for (int i = z; i > 0; i--)
    {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((x & (mask)) != 0)
        {
            ++digit;
        }
        if ((y & (mask)) != 0)
        {
            ++digit;
            ++digit;
        }
        quadKey += (digit);
    }
}

void Tile::fromQuadKey(const std::string& quadKey)
{
    x = 0; y = 0;
    z = quadKey.size();
    for (int i = z; i > 0; i--)
    {
        int mask = 1 << (i - 1);
        switch (quadKey[z - i])
        {
        case '0':
            break;

        case '1':
            x |= mask;
            break;

        case '2':
            y |= mask;
            break;

        case '3':
            x |= mask;
            y |= mask;
            break;

        default:
            GP_ERROR("Invalid QuadKey digit sequence.");
        }
    }
}