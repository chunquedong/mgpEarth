/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * Licensed under the GNU GENERAL PUBLIC LICENSE Version 3
 *
 */
#include "b3dm.h"
#include "jparser.hpp"

FE_USING_NAMESPACE

using namespace jc;

namespace detail {
    const char MAGIC[4] = { 'b', '3', 'd', 'm' };

    struct Header {
        std::uint32_t version = 1;
        std::uint32_t byteLength = size();
        std::uint32_t featureTableJSONByteLength = 0;
        std::uint32_t featureTableBinaryByteLength = 0;
        std::uint32_t batchTableJSONByteLength = 0;
        std::uint32_t batchTableBinaryByteLength = 0;

        static std::size_t size() {
            return 7 * sizeof(std::uint32_t);
        }
    };


    struct Legacy {
        enum class Type { none, json, gltf };
        Type type = Type::none;
        uint8_t buffer[4];
    };

    /** Reads b3dm header.
    */
    bool read(mgp::Stream *data, Header& header, Legacy& legacy)
    {
        char magic[4];
        data->read(magic, 4);
        
        if (std::memcmp(magic, MAGIC, sizeof(MAGIC))) {
            return false;
        }

        header.version = data->readUInt32();
        header.byteLength = data->readUInt32();
        header.featureTableJSONByteLength = data->readUInt32();
        header.featureTableBinaryByteLength = data->readUInt32();

        // OK, legacy format ends here, we have to deal with old data
        // TODO: detect old format

        header.batchTableJSONByteLength = data->readUInt32();
        header.batchTableBinaryByteLength = data->readUInt32();
        return true;
    }

    void readFeatureTable(mgp::Stream* data, Header& header, mgp::Vector3& rtcCenter)
    {
        if (!header.featureTableJSONByteLength) { return; }

        // read feature table to temporary buffer
        std::string buf;
        buf.resize(header.featureTableJSONByteLength+1);
        data->read((char*)buf.data(), header.featureTableJSONByteLength);


        JsonAllocator allocator;
        JsonParser parser(&allocator);
        Value* value0 = parser.parse((char*)buf.c_str());

        if (!value0 || parser.get_error()[0] != 0) {
            printf("parser b3dm json error: %s\n", parser.get_error());
            return;
        }

        Value* RTC_CENTER = value0->get("RTC_CENTER");
        if (RTC_CENTER)
        {
            int size = RTC_CENTER->size();
            if (size != 3) return;

            auto it = RTC_CENTER->begin();
            double v0 = it->as_float(); ++it;
            double v1 = it->as_float(); ++it;
            double v2 = it->as_float(); ++it;
            rtcCenter.set(v0, v1, v2);
        }
        //json_delete(root);
    }
}

bool TdB3dm::loadB3dm(mgp::Stream* data)
{
    detail::Header header;
    detail::Legacy legacy;
    if (!detail::read(data, header, legacy)) {
        return false;
    }

    readFeatureTable(data, header, rtcCenter);

    // ignore rest of tables
    data->seek(data->position()+header.featureTableBinaryByteLength
        + header.batchTableJSONByteLength
        + header.batchTableBinaryByteLength);

    //read gltf
    long gltfSize = data->length() - data->position();
    gltf.resize(gltfSize);
    data->read((char*)gltf.data(), gltfSize);

    return true;
}
