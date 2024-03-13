//
//  TileGeom.hpp
//  fastEarth
//
//  Created by yangjiandong on 2017/5/21.
//  Copyright © 2017年 yangjiandong. All rights reserved.
//

#ifndef TileGeom_hpp
#define TileGeom_hpp

#include "feUtil/common.h"
#include "mgp.h"
#include "feModel/Tile.h"
#include "feModel/Envelope.h"

PF_USING_NAMESPACE
FE_BEGIN_NAMESPACE

class ElevationQuery;
class PyramidGrid;

class TileGeom : public Drawable {
    UPtr<Mesh> _mesh;
    UPtr<Material> _material;

    UPtr<Texture> texture;
    Vector3 tranlation;
public:
    PyramidGrid* _pyramid;
    Vector3 getTranlation() { return tranlation; }
    Mesh* getMesh() { return _mesh.get(); }

    TileGeom(PyramidGrid* pyramid);
    ~TileGeom();
    void init(Image *image, Tile &tile, Tile* elevationTile, ElevationQuery* elevation);

    unsigned int draw(RenderInfo* view) override;
    bool doRaycast(RayQuery& query) override;
    const BoundingSphere* getBoundingSphere() override;
    Material* getMainMaterial() const override { return _material.get(); };
private:
    UPtr<Mesh> makeMesh(double radius, Vector &center, int lod, Tile &tile, Vector3& translation, Tile* elevationTile, ElevationQuery* elevation);
};


FE_END_NAMESPACE
#endif /* TileGeom_hpp */
