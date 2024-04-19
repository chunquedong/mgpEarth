//
//  TileGeom.cpp
//  mgpEarth
//
//  Created by yangjiandong on 2017/5/21.
//  Copyright © 2017年 yangjiandong. All rights reserved.
//

#include "feTile/TileGeom.hpp"
#include "mgp.h"
#include "feModel/PyramidGrid.h"
#include "feModel/GeoCoordSys.h"
#include "feTile/TileLayer.h"
#include "feElevation/Elevation.h"

FE_USING_NAMESPACE

TileGeom::TileGeom(PyramidGrid* pyramid) : _mesh(NULL), _material(NULL), texture(NULL), _pyramid(pyramid) {
    //setRenderPass(Drawable::Custom);
    _highlightType = Drawable::No;
    _pickMask = 2;
}

TileGeom::~TileGeom() {
    /*SAFE_RELEASE(_mesh);
    SAFE_RELEASE(_material);
    SAFE_RELEASE(texture);*/
}

const BoundingSphere* TileGeom::getBoundingSphere() {
    return &_mesh->getBoundingSphere();
}

static Material *makeMaterial(Material *material, Tile &tile, Image *image) {
    CF_UNUSED(tile);
    //ShaderProgram *effect = loadShaderProgram("fe.TileView");
    UPtr<Texture> texture;
    if (image) {
        texture = Texture::create(uniqueFromInstant(image));
    } else {
        texture = Texture::create("../../fastEarth/res/earth.jpg", false);
        if (!texture.get()) {
            GP_ERROR("load image error");
        }
    }
    //material->init(effect, texture);
    texture->setWrapMode(Texture::CLAMP, Texture::CLAMP, Texture::CLAMP);
    material->getParameter("u_diffuseTexture")->setValue(texture.get());
    //material->getStateBlock()->setCullFace(false);

    //SAFE_RELEASE(texture);
  
    //  if (tile.z >= 2) {
    //    material->techniques[0].passes[0].state.depthTestEnabled = false;
    //  }
    return material;
}


UPtr<Mesh> TileGeom::makeMesh(double radius, Vector &center, int lod, Tile &tile, Vector3 &translation, 
    Tile* elevationTile, ElevationQuery* elevation) {
    int latitudeBands = lod;
    int longitudeBands = lod;
  
    int vertexSize = (latitudeBands+3) * (longitudeBands+3);
    char *vertices = (char*)malloc(vertexSize * (8 * sizeof(float)));

    VertexFormat::Element elements[3];
    elements[0] = VertexFormat::Element(VertexFormat::POSITION, 3);
    elements[1] = VertexFormat::Element(VertexFormat::NORMAL, 3);
    elements[2] = VertexFormat::Element(VertexFormat::TEXCOORD0, 2);
    VertexFormat format(elements, 3);
    UPtr<Mesh> mesh = Mesh::createMesh(format, vertexSize);
  
    int vertexIndicesSize = (latitudeBands+2) * (longitudeBands+2) * 6;// +((latitudeBands + longitudeBands) * 6);
    unsigned short* indexs = (unsigned short*)malloc(vertexIndicesSize * sizeof(unsigned short));
  
  
    float *postion = (float*)vertices;
    unsigned short* indexsPosition = indexs;
  
    //------------------------------------------------------
    int w,h;
    _pyramid->tileScreenSize(w, h);
  
    Envelope mactorEnv;
    _pyramid->tileEnvelope(tile, mactorEnv);

    Envelope blEnv;
    _pyramid->tileEnvelopeBL(tile, blEnv);

    if (blEnv.minX() < -180) {
        blEnv._minX = -180;
    }
    else if (blEnv.maxX() > 180) {
        blEnv._maxX = 180;
    }

    double mactorX = mactorEnv.minX();
    double mactorY = mactorEnv.minY();
    double mactorW = mactorEnv.width();
    double mactorH = mactorEnv.height();

    double startX = (blEnv.minX());
    double startY = (blEnv.minY());
    double width = (blEnv.width());
    double height = (blEnv.height());

    if (startY+height > 85)
        height = 90-startY;

    if (startY < -85) {
        height = height + (90 + startY);
        startY = -90;
    }

    double skirtPadding = width*0.05;

    //center point offset
    Coord2D centerBL(startX+width/2, startY+height/2);
    Vector localCenter;
    GeoCoordSys::blToXyz(centerBL, localCenter, radius);
    translation = (center + localCenter);
  
    for (int i=-1; i <= latitudeBands+1; i++) {
        bool ySkirt = false;
        double latitude;
        if (i < 0) {
            latitude = startY - skirtPadding;
            ySkirt = true;
        }
        else if (i > latitudeBands) {
            latitude = height+startY + skirtPadding;
            ySkirt = true;
        }
        else {
            latitude = i * height / latitudeBands + startY;
        }

        for (int j=-1; j <= longitudeBands+1; j++) {
            bool xSkirt = false;
            double longitude;
            if (j < 0) {
                longitude = startX - skirtPadding;
                xSkirt = true;
            }
            else if (j > latitudeBands) {
                longitude = width + startX + skirtPadding;
                xSkirt = true;
            }
            else {
                longitude = j * width / longitudeBands + startX;
            }

      
            Coord2D coordBL(longitude, latitude);
            //coord now is mercator
            Coord2D coordMercator = coordBL;
            GeoCoordSys::earth()->toMercator(&coordMercator);

            //must calc texture coord in mercator
            double u = /*longNumber /(double)longitudeBands;*/ (coordMercator.x - mactorX) / mactorW;
            double v;
            if (latitude == 90) {
                v = 0;
                coordMercator.y = 20037508.34;
            }
            else if (latitude == -90) {
                v = 1.0;
                coordMercator.y = -20037508.34;
            }
            else {
                v = /*1-(latNumber / (double)latitudeBands);*/ 1 - (coordMercator.y - mactorY) / mactorH;
            }

            double height = 0;
            if (elevation) {
                height = elevation->getTileHeight(*elevationTile, coordMercator.x, coordMercator.y, 1, tile.z);
            }
            if (xSkirt || ySkirt) {
                height -= mactorW * 0.2;
            }

            Vector vector;
            GeoCoordSys::blToXyz(coordBL, vector, radius+ height);
      
            Vector pos = vector + center - translation;
            postion[0] = pos.x;
            postion[1] = pos.y;
            postion[2] = pos.z;

            //TODO: normal
            postion[3] = 0;
            postion[4] = 0;
            postion[5] = 0;

            postion[6] = u;
            postion[7] = v;
            postion += 8;
        }
    }
  
    for (int i=0; i < latitudeBands+2; i++) {
        for (int j=0; j < longitudeBands+2; j++) {
            int second = (i * (longitudeBands + 3)) + j;
            int first = second + longitudeBands + 3;
            //set index
            indexsPosition[0] = first;
            indexsPosition[1] = second;
            indexsPosition[2] = first + 1;
            indexsPosition[3] = second;
            indexsPosition[4] = second + 1;
            indexsPosition[5] = first + 1;
            indexsPosition += 6;
        }
    }

    mesh->getVertexBuffer()->setData(vertices, vertexSize * (8 * sizeof(float)));
    mesh->addPart(Mesh::TRIANGLES, vertexIndicesSize);
    mesh->getIndexBuffer()->setData((char*)indexs, vertexIndicesSize * sizeof(unsigned short));
    return mesh;
}

bool TileGeom::doRaycast(RayQuery& query) {
    return _mesh->doRaycast(query);
}

void TileGeom::init(Image *image, Tile &tile, Tile* elevationTile, ElevationQuery* elevation) {
    if (_mesh.get()) return;
    //inited = true;
    //Envelope envelope;
    //PyramidGrid::getDefault()->tileEnvelopeBL(tile, envelope);
  
    //adjust the radius
    double radius = GeoCoordSys::earth()->getRadius();

    if (tile.z < 10) {
        int num = 1 << tile.z;
        double scale = (1.0 - (0.01 / num));
        radius *= scale;
    }

    Vector center;
    center.set(0,0,0);
  
    //get the level of details
    int lod = 20;
 
    _mesh = makeMesh(radius, center, lod, tile, tranlation, elevationTile, elevation);
  
    _material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag");
    makeMaterial(_material.get(), tile, image);
}

unsigned int TileGeom::draw(RenderInfo* view)
{
    if (!_mesh.get()) return 0;
    _mesh->draw(view, this, _material.get(), NULL, 0);

    return 1;
}
