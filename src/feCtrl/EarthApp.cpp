#include "feCtrl/EarthApp.h"

//#include "pfRender/GlRenderer.h"
//#include "pfApp/Application.h"
#include "feTile/TileLayer.h"
#include "feModel/GeoCoordSys.h"
//#include "pf2d/DrawNode.h"
#include "feCtrl/TraceBall.h"
//#include "pfLoader/AssimpLoader.h"
#include "feModel/PyramidGrid.h"
#include "fe3dtiles/TdTilesManager.h"
#include "feElevation/Elevation.h"
#include "feGeo/GeoLayer.h"
#include "feObjects/GroundNode.h"
#include "feObjects/Skybox.h"

FE_USING_NAMESPACE

extern OfflineElevation* g_defaultOfflineElevation;

EarthApp::EarthApp() : earth(nullptr)
{
    printf("mgpEarth 1.0\n");
    font = Font::create("res/ui/sans.ttf");
    earthCtrl = new EarthCtrl();
    gesture.listener = earthCtrl;
    earthCtrl->picker.listener = this;
    g_defaultOfflineElevation = new OfflineElevation("res/fastEarth/gm_el_v1_small.png");
}

EarthApp::~EarthApp()
{
    _setRefCount(0);
    earthCtrl->release();
}

void EarthApp::finalize() {
    font.clear();
    earthCtrl->finalize();
    Application::finalize();
    _progressView = nullptr;
}

void EarthApp::drawLocationText() {
    _font->start();
    Rectangle* viewport = getView()->getViewport();
    int padding = 10;
    int fontSize = 13;
    float y = viewport->height / Toolkit::cur()->getScreenScale() - fontSize - padding;
    float x = viewport->width / Toolkit::cur()->getScreenScale();
    char buffer[256] = { 0 };
    Coord2D pos = earthCtrl->getPosition();
    snprintf(buffer, 256, "%f, %f, %d", pos.x, pos.y, (int)earthCtrl->getZoom());

    unsigned int tw, th;
    _font->measureText(buffer, fontSize, &tw, &th, -1);
    x -= (tw+ padding+ padding);

    _font->drawText(buffer, x, y, Vector4::one(), fontSize);
    _font->finish(NULL);
}

void EarthApp::render(float elapsedTime) {
    Application::render(elapsedTime);

    earthCtrl->picker.render(getView()->getCamera(), getView()->getViewport());

    font->start();
    Rectangle* viewport = getView()->getViewport();
    int padding = 10;
    int fontSize = 13;
    float y = viewport->height / Toolkit::cur()->getScreenScale() - fontSize - padding;
    font->drawText(L"mgpEarth", padding, y, Vector4::one(), fontSize, wcslen(L"mgpEarth"));
    font->finish(NULL);

    drawLocationText();
}

void EarthApp::update(float elapsedTime) {

    earthCtrl->updateCamera(elapsedTime , *getView()->getCamera(), *getView()->getViewport());
    Application::update(elapsedTime);

    bool visiable = earthCtrl->getDistanceToSurface() < 400000;
    if (atmosphere) atmosphere->getDrawable()->setVisiable(!visiable);

    updateSkybox();
}

void EarthApp::addGeoNode(UPtr<GeoNode> node) {
    node->ctrl = getEarthCtrl();
    getView()->getScene()->addNode(std::move(node));
}

static void addTestMesh(Scene* _scene, float r)
{
#if 1
    // Create 3 vertices. Each vertex has position (x, y, z) and color (red, green, blue)
    float vertices[] =
    {
        0, 0, 0,     1.0f, 0.0f, 0.0f,
        r, 0, 0,     1.0f, 0.0f, 0.0f,

        0, 0, 0,     0.0f, 1.0f, 0.0f,
        0, r, 0,     0.0f, 1.0f, 0.0f,

        0, 0, 0,     0.0f, 0.0f, 1.0f,
        0, 0, r,     0.0f, 0.0f, 1.0f,
    };
    unsigned int vertexCount = 6;
    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        VertexFormat::Element(VertexFormat::COLOR, 3)
    };
    UPtr<Mesh> mesh = Mesh::createMesh(VertexFormat(elements, 2), vertexCount);
    if (mesh.get() == NULL)
    {
        GP_ERROR("Failed to create mesh.");
        return;
    }
    mesh->setPrimitiveType(Mesh::LINES);
    mesh->getVertexBuffer()->setData((char*)vertices, sizeof(vertices));

    UPtr<Model> _model = Model::create(std::move(mesh));

    Material* material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag", "VERTEX_COLOR");
    material->getStateBlock()->setCullFace(false);

    Node* modelNode = _scene->addNode("axis");
    modelNode->setDrawable(_model.dynamicCastTo<Drawable>());
#else
    UPtr<Model> _model = Model::create(Mesh::createSpherical());
    Material* material = _model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag");
    material->getParameter("u_diffuseColor")->setVector4(Vector4(0.0, 0.8, 1.0, 1.0));

    Node* modelNode = _scene->addNode("test");
    modelNode->setDrawable(_model.dynamicCastTo<Drawable>());
    modelNode->setTranslation(-16679.187500000000, -379663.18750000000, -191802.21875000000);
    modelNode->scale(1671194.7448840307);
#endif
}

void EarthApp::initialize() {
    UPtr<Scene> scene = Scene::create();
    UPtr<Camera> camera = Camera::createPerspective(45, 1, 1, 100000);
    Node* cameraNode = scene->addNode("camera");
    getView()->setCamera(camera.get(), false);
    cameraNode->setCamera(std::move(camera));
    cameraNode->translateZ(GeoCoordSys::earth()->getRadius()*2);
    getView()->setScene(std::move(scene));

    addAtmosphere();

#if _DEBUG
    addTestMesh(getView()->getScene(), 6378137.0 * 2);
#endif // _DEBUG


    earthCtrl->_sceneView = getView();

    earthCtrl->picker.init(Renderer::cur());
}

void EarthApp::addAtmosphere() {
    UPtr<GradientSphere> _model(new GradientSphere());
    _model->setPickMask(0);
    Node* modelNode2 = getView()->getScene()->addNode("atmosphere");
    modelNode2->setDrawable(_model.dynamicCastTo<Drawable>());
    modelNode2->scale(GeoCoordSys::earth()->getRadius() + 100000);
    atmosphere = modelNode2;
}

void EarthApp::updateSkybox() {
    if (!_skybox) return;
    
    Vector out = getView()->getCamera()->getNode()->getTranslationWorld();
    Matrix m;
    Matrix::createLookAt(out, Vector3::zero(), Vector3::unitZ(), &m, false);
    Matrix rm;
    rm.rotateX(MATH_DEG_TO_RAD(90));
    _skybox->setMatrix(m * rm);
}

void EarthApp::addSkybox(int dark, double minDis, double maxDis) {
    std::vector<std::string> faces;
    if (dark == 0) {
        faces = {
            "res/skybox/skyboxsun25deg/nx.jpg",
            "res/skybox/skyboxsun25deg/px.jpg",
            "res/skybox/skyboxsun25deg/py.jpg",
            "res/skybox/skyboxsun25deg/ny.jpg",
            "res/skybox/skyboxsun25deg/nz.jpg",
            "res/skybox/skyboxsun25deg/pz.jpg"
        };
    }
    else if (dark == 1) {
        faces = {
            "res/skybox/MilkyWay/dark-s_nx.jpg",
            "res/skybox/MilkyWay/dark-s_px.jpg",
            "res/skybox/MilkyWay/dark-s_py.jpg",
            "res/skybox/MilkyWay/dark-s_ny.jpg",
            "res/skybox/MilkyWay/dark-s_pz.jpg",
            "res/skybox/MilkyWay/dark-s_nz.jpg"
        };
    }
    UPtr<SkyBox> skybox(new SkyBox(faces));
    if (dark) {
        skybox->setName("darkSkybox");
    }
    else {
        skybox->setName("skybox");
    }
    skybox->maxDis = maxDis;
    skybox->minDis = minDis;
    if (minDis < 1000) {
        _skybox = skybox.get();
        skybox->followCamera = false;
    }
    else {
        skybox->followCamera = true;
    }
    addGeoNode(std::move(skybox));
}

bool EarthApp::removeNode(const char* name) {
    Node *node = getView()->getScene()->findNode(name);
    if (node) {
        if (node == atmosphere) {
            atmosphere = NULL;
        }
        else if (node == _skybox) {
            _skybox = NULL;
        }
        else if (node == earth) {
            earth = NULL;
            earthCtrl->_groundNode = NULL;
        }
        node->remove();
        return true;
    }
    return false;
}

XyzTileManager* EarthApp::addTileLayer(const char *name, const char* uri, const char* elevationUri) {
    std::string path = "tile/";
    XyzTileManager* tileManager = new XyzTileManager(uri);
    tileManager->setCachePath(path+name);

    if (elevationUri) {
        elevation = SPtr<ElevationManager>(new ElevationManager(elevationUri));
        //elevation->elevationScale = 2;
        elevation->setCachePath(path+"terrain-rgb");
        tileManager->setElevation(elevation.get());
    }
    TileLayer* lyr = new TileLayer(tileManager);
    lyr->setName(name);

    //view = Node::create(name);
    //view->addComponent(lyr);
    getView()->getScene()->insertNode(UPtr<Node>(lyr));

    earth = lyr;
    earthCtrl->_groundNode = earth;
    return tileManager;
}

Node* EarthApp::add3dtiles(const char* name, const char* uri, const Coord2D& coord, double height, bool lighting)
{
    TdTilesManager* tdtiles = new TdTilesManager(uri);
    tdtiles->lighting = lighting;
    TileLayer* layer = new TileLayer(tdtiles);
    layer->setName(name);
    getView()->getScene()->addNode(UPtr<Node>(layer));

    Vector out;
    GeoCoordSys::blToXyz(coord, out, GeoCoordSys::earth()->getRadius()+ height);

    Matrix m;
    Matrix::createLookAt(out, Vector3::zero(), Vector3::unitZ(), &m, false);
    layer->setMatrix(m);
    //layer->rotateX(MATH_PI / 4);
    //layer->scale(10);
    return layer;
}

bool EarthApp::mouseEvent(MotionEvent &event)
{
    if (Application::mouseEvent(event)) {
        return true;
    }
    gesture.onPress(&event);
    return true;
}

bool EarthApp::onPickNode(PickResult& pickResult) {
    return true;
}

bool EarthApp::showLoadProgress(Node* node) {
    if (!_progressView) {
        UPtr<Form> form = Form::create();
        form->getContent()->setAutoSizeW(Control::AUTO_WRAP_CONTENT);
        form->getContent()->setAutoSizeH(Control::AUTO_WRAP_CONTENT);
        form->getContent()->setPadding(20, 20, 20, 20);
        form->getContent()->setLayout(Layout::LAYOUT_ABSOLUTE);
        form->getContent()->setAlignment(Control::ALIGN_VCENTER_HCENTER);

        UPtr<ProgressBar> progressBar = Control::create<ProgressBar>("ProgressBar");
        progressBar->setWidth(300);
        progressBar->setValue(0);
        form->getContent()->addControl(std::move(progressBar));

        _progressView = form.get();
        getFormManager()->add(std::move(form));
    }
    //_progressView->setVisiable(true);
    ProgressBar* progressBar = dynamic_cast<ProgressBar*>(_progressView->getContent()->findControl("ProgressBar"));
    
    float progress = 0;
    if (GeoNode* geoNode = dynamic_cast<GeoNode*>(node)) {
        progress = geoNode->getProgress();
    }
    else if (TileLayer* layerNode = dynamic_cast<TileLayer*>(node)) {
        progress = layerNode->getProgress();
    }
    else {
        return false;
    }
    progressBar->setValue(progress);

    _checkProgress = [=]() {
        float progress = 0;
        if (GeoNode* geoNode = dynamic_cast<GeoNode*>(node)) {
            progress = geoNode->getProgress();
        }
        else if (TileLayer* layerNode = dynamic_cast<TileLayer*>(node)) {
            progress = layerNode->getProgress();
        }
        if (progress >= 1.0) {
            setTimeout(200, [=]() {
                getFormManager()->remove(_progressView);
                this->_progressView = nullptr;
                //_progressView->setVisiable(false);
            });
        }
        else {
            setTimeout(200, this->_checkProgress);
        }
        progressBar->setValue(progress);
    };
    setTimeout(200, _checkProgress);
    return true;
}

bool EarthApp::getPickResult(RayQuery& result, PickResult& pickResult) {
    if (result.drawable && result.drawable->getNode()) {
        int drawableIndex = 0;
        if (result.path.size() > 0) {
            drawableIndex = result.path[0];
        }
        Node* layer = NULL;
        std::string path;
        long userId = -1;

        for (Node* node = result.drawable->getNode(); node != NULL; node = node->getParent()) {
            if (node->getParent() == NULL) {
                //root
                break;
            }

            GeoNode* geoNode = dynamic_cast<GeoNode*>(node);
            if (geoNode) {
                layer = geoNode;
            }
            if (const char* userIds = node->getTag("user_id")) {
                userId = atol(userIds);
            }

            path = std::string(node->getName()) + "/" + path;
        }

        if (!layer) {
            layer = result.drawable->getNode();
        }
        pickResult.layer = layer;
        pickResult.path = path;
        pickResult.userId = userId;
        pickResult.drawable = result.drawable;
        pickResult.drawableIndex = drawableIndex;
        return true;
    }
    return false;
}

bool EarthApp::onPick(float x, float y, RayQuery& result) {
    PickResult pickResult;
    if (getPickResult(result, pickResult)) {
        return onPickNode(pickResult);
    }
    return false;
}
