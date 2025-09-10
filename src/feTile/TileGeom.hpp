//
//  TileGeom.hpp
//  mgpEarth
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

FE_BEGIN_NAMESPACE

class ElevationQuery;
class PyramidGrid;

class TileGeom : public mgp::Drawable {
    mgp::UPtr<mgp::Mesh> _mesh;
    mgp::UPtr<mgp::Material> _material;

    mgp::UPtr<mgp::Texture> texture;
    mgp::Vector3 tranlation;
public:
    PyramidGrid* _pyramid;
    mgp::Vector3 getTranlation() { return tranlation; }
    mgp::Mesh* getMesh() { return _mesh.get(); }

    TileGeom(PyramidGrid* pyramid);
    ~TileGeom();
    void init(mgp::Image *image, Tile &tile, Tile* elevationTile, ElevationQuery* elevation);

    unsigned int draw(mgp::RenderInfo* view) override;
    bool doRaycast(mgp::RayQuery& query) override;
    const mgp::BoundingSphere* getBoundingSphere() override;
    mgp::Material* getMainMaterial() const override { return _material.get(); };
private:
    mgp::UPtr<mgp::Mesh> makeMesh(double radius, mgp::Vector3 &center, int lod, Tile &tile, mgp::Vector3& translation, Tile* elevationTile, ElevationQuery* elevation);
};


FE_END_NAMESPACE
#endif /* TileGeom_hpp */
