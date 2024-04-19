/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of mgpEarth project
 * all rights reserved
 *
 */
#include "Building.h"
#include "feModel/GeoCoordSys.h"
#include "jparser.hpp"

using namespace jc;
FE_USING_NAMESPACE

Building::Building(const char* uri): GeoNode(uri) {
    color = Vector4(51 / 255.0, 153 / 255.0, 255 / 255.0, 1.0);
    request->setSaveToFile(false);
}
Building::~Building() {
}

static void decodeBuilding(MeshBatch* mesh, Stream* stream, Vector3& translation, float heightScale) {
    int code = stream->readUInt32();
    if (code != 0x9abc) {
        printf("building format error\n");
        return;
    }
    int version = stream->readUInt32();
    int count = stream->readUInt32();
    stream->readUInt32();
    translation.x = stream->readDouble();
    translation.y = stream->readDouble();
    translation.z = stream->readDouble();

    std::vector<uint32_t> indexBuf0;
    std::vector<float> vertexBuf0;
    std::vector<uint32_t> indexBuf;
    std::vector<float> vertexBuf;

    Vector3 up = translation;
    up.normalize();
    for (int i = 0; i < count; ++i) {
        uint8_t vertexCount = stream->readUInt8();
        float height = stream->readFloat() * heightScale;

        vertexBuf0.clear();
        vertexBuf0.resize(vertexCount * 3);
        stream->read((char*)vertexBuf0.data(), vertexCount * 3 * sizeof(float));

        uint8_t indexCount = stream->readUInt8();
        indexBuf0.clear();
        indexBuf0.resize(indexCount * sizeof(uint32_t));
        stream->read((char*)indexBuf0.data(), indexCount, sizeof(uint32_t));

        indexBuf.clear();
        vertexBuf.clear();
        for (int j = 0; j < vertexBuf0.size(); j+=3) {
            float x = vertexBuf0[j];
            float y = vertexBuf0[j+1];
            float z = vertexBuf0[j+2];
            vertexBuf.push_back(x-up.x * 10);
            vertexBuf.push_back(y-up.y * 10);
            vertexBuf.push_back(z-up.z * 10);
            vertexBuf.push_back(0);

            vertexBuf.push_back(x+up.x * height);
            vertexBuf.push_back(y+up.y * height);
            vertexBuf.push_back(z+up.z * height);
            vertexBuf.push_back(height);
        }

        for (int j = 0; j < vertexCount; ++j) {
            int base = j*2;
            if (j == vertexCount -1) {
                indexBuf.push_back(base + 0);
                indexBuf.push_back(0);
                indexBuf.push_back(base + 1);
                indexBuf.push_back(base + 1);
                indexBuf.push_back(0);
                indexBuf.push_back(1);
                break;
            }
            indexBuf.push_back(base + 0);
            indexBuf.push_back(base + 2);
            indexBuf.push_back(base + 1);
            indexBuf.push_back(base + 1);
            indexBuf.push_back(base + 2);
            indexBuf.push_back(base + 3);
        }

        for (int j = 0; j < indexCount; ++j) {
            indexBuf.push_back(indexBuf0[j] * 2 + 1);
        }

        mesh->add(vertexBuf.data(), vertexCount*2, indexBuf.data(), indexBuf.size());
    }
}

void* Building::decodeFile(const char* path, NetResponse& res, MultiRequest* req) {
    if (res.result.size() == 0) {
        return NULL;
    }

    UPtr<Material> material = Material::create("res/shaders/custom/building.vert", "res/shaders/custom/building.frag");
    // Set initial material state
    material->getParameter("u_diffuseColor")->setVector4(color);

    VertexFormat::Element elements[] =
    {
        VertexFormat::Element(VertexFormat::POSITION, 3),
        //VertexFormat::Element(VertexFormat::NORMAL, 3),
        VertexFormat::Element("a_height", 1),
    };
    VertexFormat vertexFormat(elements, 2);
    UPtr<MeshBatch> mesh = MeshBatch::create(vertexFormat, Mesh::TRIANGLES, std::move(material), Mesh::INDEX32, 10*1024*1024, 10 * 1024*1024);
    Vector3 translation;
    UPtr<Stream> stream(new Buffer((uint8_t*)res.result.data(), res.result.size(), false));
    decodeBuilding(mesh.get(), stream.get(), translation, heightScale);

    mesh->setRenderLayer(Drawable::Custom);

    Node* node = Node::createForComponent(std::move(mesh)).take();
    node->setTranslation(translation);
    return node;
}

static void parserColor(Value* json, const char* name, Vector4& color) {
    Value* lineColor = json->get(name);
    if (lineColor && lineColor->size() == 4) {
        auto c = lineColor->begin();
        color.x = c->as_float();
        ++c;
        color.y = c->as_float();
        ++c;
        color.z = c->as_float();
        ++c;
        color.w = c->as_float();
        //++c;
    }
}

bool Building::loadOptions(char* json_str) {
    JsonAllocator allocator;
    JsonParser parser(&allocator);
    Value* value0 = parser.parse(json_str);

    if (!value0 || parser.get_error()[0] != 0) {
        printf("parser options json error: %s\n", parser.get_error());
        return false;
    }

    Value* max = value0->get("maxDis");
    if (max) {
        this->maxDis = max->as_float();
    }

    Value* min = value0->get("minDis");
    if (min) {
        this->minDis = min->as_float();
    }

    parserColor(value0, "color", this->color);
    return true;
}