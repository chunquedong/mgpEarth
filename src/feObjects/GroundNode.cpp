#include "GroundNode.h"
#include "feCtrl/EarthCtrl.h"
#include "feModel/GeoCoordSys.h"
#include "feCtrl/EarthApp.h"

FE_USING_NAMESPACE

GroundModel::GroundModel(const char* uri): GltfNode(uri), lastUpdateTime(0), height(0), dirty(true), updateDelay(1000) {
}

void GroundModel::setPosition(const Coord2D& p, double height) {
    if (position.x != p.x || position.y != p.y || this->height != height) {
        dirty = true;
    }
    position = p;
    this->height = height;
}

bool GroundModel::updateHeight(int stickMethod) {
    if (stickMethod == 1) {
        Vector3 target;
        if (app && EarthCtrl::getGroundPoint(position, app->getEarth(), target)) {
            Vector earthPosition;
            GeoCoordSys::blToXyz(position, earthPosition, GeoCoordSys::earth()->getRadius());
            height = earthPosition.distance(target);
            return true;
        }
        else {
            return false;
        }
    }
    else if (stickMethod == 2) {
        if (!OfflineElevation::cur()) return false;
        height = OfflineElevation::cur()->getHeight(position.x, position.y, 18);
        return true;
    }
    return true;
}

void GroundModel::update(float elapsedTime) {
    GeoNode::update(elapsedTime);
    if (!dirty) return;
    if (loadState != 2) return;

    if (autoStickGround) {
        uint64_t now = System::millisTicks();
        if (now - lastUpdateTime > updateDelay) {
            if (updateHeight(autoStickGround)) {
                lastUpdateTime = now;
            }
        }
    }

    Vector3 target;
    GeoCoordSys::blToXyz(position, target, GeoCoordSys::earth()->getRadius()+ height);

    Vector3 dir; target.normalize(&dir);
    Matrix lookAtMatrix;
    Matrix::createLookAt(target + dir * 100, Vector3::zero(), Vector3::unitZ(), &lookAtMatrix, false);

    Matrix rotateMatrix;
    if (!direction.isZero()) {
        Vector3 up = Vector3::unitY();
        Vector3 cross; Vector3::cross(direction, up, &cross);
        Matrix::createRotation(cross, -Vector3::angle(direction, up), &rotateMatrix);
    }
    this->setMatrix(lookAtMatrix * rotateMatrix * pose);

    dirty = false;
}


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

TrackModel::TrackModel(): direction(Vector3::unitZ()), updateDelay(1000), lastUpdateTime(0), _id(-1) {
}

TrackModel::~TrackModel()
{
}

void TrackModel::setNode(Node* node) {
    _node = node;
}

void TrackModel::setFromLonLat(std::vector<Coord2D>& path2d, double height) {
    reset();
    path.clear();

    Vector3 target;
    for (auto position : path2d) {
        //double lastGroundHeight = OfflineElevation::cur()->getHeight(position.x, position.y, 18);
        GeoCoordSys::blToXyz(position, target, GeoCoordSys::earth()->getRadius() + height);
        path.push_back(target);
    }
}

bool TrackModel::updateHeight(int stickMethod) {
    if (stickMethod == 1) {
        Vector3 target;
        if (parent->app && EarthCtrl::getGroundPointByNormal(_curPosition, parent->app->getEarth(), target)) {
            _curPosition = target;
            _height = _curPosition.length() - GeoCoordSys::earth()->getRadius();
            return true;
        }
        else {
            return false;
        }
    }
    else if (stickMethod == 2) {
        if (!OfflineElevation::cur()) return false;
        Coord2D lnglat;
        GeoCoordSys::xyzToBl(_curPosition, lnglat);
        double height = OfflineElevation::cur()->getHeight(lnglat.x, lnglat.y, 18);
        Vector3 normal;
        _curPosition.normalize(&normal);
        _curPosition = normal * (GeoCoordSys::earth()->getRadius() + height);
        _height = height;
        return true;
    }
    return true;
}

bool TrackModel::tryUpdateHeight() {
    if (autoStickGround) {
        uint64_t now = System::millisTicks();
        if (now - lastUpdateTime > updateDelay) {
            if (updateHeight(autoStickGround)) {
                lastUpdateTime = now;
                return true;
            }
        }
        else {
            Vector3 normal;
            _curPosition.normalize(&normal);
            _curPosition = normal * (GeoCoordSys::earth()->getRadius() + _height);
        }
    }
    return false;
}

void TrackModel::update(float elapsedTime) {
    //GltfNode::update(elapsedTime);
    if (!_node) {
        return;
    }

    if (beforeDelayTime != 0 && startTime != 0) {
        uint64_t now = System::currentTimeMillis();
        if (now - startTime < beforeDelayTime) {
            return;
        }
        startTime = 0;
    }

    if (pathEndTime != 0) {
        uint64_t now = System::currentTimeMillis();
        if (now - pathEndTime > afterDelayTime) {
            pathEndTime = 0;
            if (onEnd) {
                onEnd(this);
            }
        }
    }

    if (!_isRuning) {
        if (lastUpdateTime == 0) {
            tryUpdateHeight();
        }
        return;
    }

    if (path.size() == 0) return;

    if (path.size() == 1) {
        _curPosition = path[0];
        tryUpdateHeight();

        //Vector3 dir; target.normalize(&dir);
        Matrix lookAtMatrix;
        Matrix::createLookAt(_curPosition, Vector3::zero(), direction, &lookAtMatrix, false);
        _node->setMatrix(lookAtMatrix * pose);

        if (onPositionUpdate) {
            onPositionUpdate(this);
        }
        setStop();
        return;
    }

    int beginSegemntIndex = lastPointIndex;
    double beginSegmentOffset = segmentOffset;

    double advance = elapsedTime / 1000.0 * speed;
    Vector3 lineDirection = this->direction;
    bool ok = false;
    while (lastPointIndex + 1 < path.size()) {
        Vector3& p0 = path[lastPointIndex];
        Vector3& p1 = path[lastPointIndex + 1];
        double segmentLength = p0.distance(p1);

        if (segmentOffset + advance < segmentLength) {
            segmentOffset += advance;
            Vector3 dir = p1 - p0;
            Vector3 p = p0 + dir * (segmentOffset / segmentLength);
            _curPosition = p;
            lineDirection = dir.normalize();
            ok = true;
            _offsetLength += advance;

            break;
        }
        else {
            advance -= (segmentLength - segmentOffset);
            segmentOffset = 0;
            lastPointIndex++;
            _offsetLength += segmentLength;
        }
    }

    if (!ok) {
        setStop();
        pathEndTime = System::currentTimeMillis();
        _curPosition = path[path.size() - 1];
    }

    tryUpdateHeight();

    //skip line segment or first init
    if (lastPointIndex - beginSegemntIndex > 1 || (beginSegemntIndex == 0 && beginSegmentOffset == 0) ) {
        direction = lineDirection;
    }
    else {
        double blendAlpha = 0.5;
        direction = this->direction * (1.0- blendAlpha) + lineDirection * blendAlpha;
    }

    Matrix lookAtMatrix;
    Matrix::createLookAt(_curPosition, Vector3::zero(), direction, &lookAtMatrix, false);

    _node->setMatrix(lookAtMatrix * pose);

    if (onPositionUpdate) {
        onPositionUpdate(this);
    }
}
void TrackModel::start() {
    _isRuning = true;
    startTime = System::currentTimeMillis();
}
void TrackModel::stop() {
    setStop();
    reset();
}
void TrackModel::reset() {
    lastPointIndex = 0;
    segmentOffset = 0;
    pathEndTime = 0;
    _offsetLength = 0;
}

void TrackModel::pause()
{
    setStop();
}

bool TrackModel::isRuning() {
    return _isRuning && _node;
}

void TrackModel::playAnimation(int repeatCount)
{
    if (!this->getNode()) {
        return;
    }
    
    std::set<Animation*> animations;
    this->getNode()->getAllAnimations(animations);
    for (auto it = animations.begin(); it != animations.end(); ++it) {
        Animation* anim = *it;
        AnimationClip* clip = anim->getClip();
        if (clip->isPlaying()) {
            continue;
        }
        clip->setRepeatCount(repeatCount);
        clip->play();
    }
}

void TrackModel::setStop()
{
    if (_isRuning) {
        _isRuning = false;

        if (onStop)
            onStop(this);
    }
}

void TrackModel::stopAnimation() {
    std::set<Animation*> animations;
    this->getNode()->getAllAnimations(animations);
    for (auto it = animations.begin(); it != animations.end(); ++it) {
        Animation* anim = *it;
        AnimationClip* clip = anim->getClip();
        clip->stop();
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

MultiModel::MultiModel(const char* uri) : GltfNode(uri) {
}

void MultiModel::update(float elapsedTime) {
    for (auto it = _instances.begin(); it != _instances.end(); ++it) {
        TrackModel* model = it->second.get();
        if (_templateModel.get() && model->getNode() == nullptr) {
            UPtr<Node> node = _templateModel->clone();// GltfModel::makeTemplateInstance(_templateModel.get());

            //rebind skin
            std::vector<Drawable*> list;
            node->getAllDrawable(list);
            for (Drawable* drawable : list) {
                Model* model = dynamic_cast<Model*>(drawable);
                if (model && model->getSkin()) {
                    model->getSkin()->bindNode(node.get());
                }
            }

            //std::string id = std::to_string(model->_id);
            node->setUserId(model->_id);
            model->setNode(node.get());
            this->addChild(std::move(node));
        }
        model->update(elapsedTime);
    }
    GeoNode::update(elapsedTime);
}

int MultiModel::add(UPtr<TrackModel> inst) {
    int id = inst->_id;
    if (id == -1) {
        id = ++_idCount;
        inst->_id = id;
    }
    inst->parent = this;
    _instances[id] = std::move(inst);
    return id;
}
void MultiModel::remove(int id) {
    auto it = _instances.find(id);
    if (it != _instances.end()) {
        Node* node = it->second->getNode();
        if (node) {
            node->remove();
        }
        _instances.erase(it);
    }
}
TrackModel* MultiModel::get(int id) {
    auto it = _instances.find(id);
    if (it == _instances.end()) {
        return nullptr;
    }
    return it->second.get();
}

void MultiModel::clear()
{
    for (auto it = _instances.begin(); it != _instances.end(); ++it) {
        Node* node = it->second->getNode();
        if (node) {
            node->remove();
        }
    }
    _instances.clear();
}

void MultiModel::onReceive(Task* task, NetResponse& res, MultiRequest* req) {
    if (res.decodeResult) {
        Node* node = (Node*)res.decodeResult;
        _templateModel = (UPtr<Node>(node));
        loadState = 2;
        this->setBoundsDirty();
    }
}
