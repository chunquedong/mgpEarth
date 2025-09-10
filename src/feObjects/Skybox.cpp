#include "Skybox.h"

FE_USING_NAMESPACE
PF_USING_NAMESPACE

SkyBox::SkyBox(std::vector<std::string>& faces) : GeoNode("") {
    this->faces.swap(faces);
}

void SkyBox::load() {
    UPtr<CubeMap> cube = UPtr<CubeMap>(new CubeMap());
    cube->load(faces, true);
    cube->followCamera = followCamera;
    this->setDrawable(cube.dynamicCastTo<Drawable>());
}

