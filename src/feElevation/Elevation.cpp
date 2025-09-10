#include "Elevation.h"
#include "mgp.h"
//#include "cppfan/Profiler.h"
#include "feTile/TileData.h"
#include "feTile/TileGeom.hpp"
#include "feModel/PyramidGrid.h"
#include "feModel/GeoCoordSys.h"

#include "3rd/stb_image.h"

FE_USING_NAMESPACE

OfflineElevation* g_defaultOfflineElevation;

OfflineElevation* OfflineElevation::cur() {
    return g_defaultOfflineElevation;
}
void OfflineElevation::_setCur(OfflineElevation* elev) {
    g_defaultOfflineElevation = elev;
}

OfflineElevation::OfflineElevation(): imageData(NULL), width(0), height(0), elevationScale(1), format(-1) {
    envelope.init(-180, -90, 180, 90);
}

OfflineElevation::OfflineElevation(const char* uri, int format, Envelope* env) : elevationScale(1) {
    if (!env) {
        envelope.init(-180, -90, 180, 90);
    }
    else {
        envelope = *env;
    }

    // int max = INT_MIN;
    // int min = INT_MAX;

    int iw, ih, n;
    void* data = 0;
    if (format == 2) {
        stbi_set_flip_vertically_on_load(false);
        data = stbi_load_16(uri, &iw, &ih, &n, 0);

        // for (int i = 0; i < iw; ++i) {
        //     for (int j = 0; j < ih; ++j) {
        //         uint16_t* pos = (uint16_t*)data + ((iw * j + i) * n);
        //         double v = *pos;
        //         if (v > max) max = v;
        //         if (v < min) min = v;
        //     }
        // }
    }
    else if (format == 3) {
        stbi_set_flip_vertically_on_load(false);
        data = stbi_load(uri, &iw, &ih, &n, 0);
        GP_ASSERT(n == 3);

        // for (int i = 0; i < iw; ++i) {
        //     for (int j = 0; j < ih; ++j) {
        //         uint8_t* pos = (uint8_t*)data + ((iw * j + i) * n);
        //         int R = pos[0];
        //         int G = pos[1];
        //         int B = pos[2];
        //         if (R == 6) {
        //             R = 1;
        //         }
        //         double v = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1);
        //         if (v > max) max = v;
        //         if (v < min) min = v;
        //     }
        // }
    }
    else {
        printf("Unknow format:%d\n", format);
        abort();
    }

    if (!data) {
        printf("load image error:%s\n", uri);
        abort();
    }

    this->format = format;
    imageData = data;
    width = iw;
    height = ih;
}

double OfflineElevation::sampleHeight(int x, int y) {
    if (x >= width) x = width - 1;
    if (y >= height) y = height - 1;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    double heightv = 0;
    if (format == 3) {
        int ppb = 3;
        uint8_t* data = (uint8_t*)imageData;
        uint8_t* pos = data + ((width * y + x) * ppb);
        int R = pos[0];
        int G = pos[1];
        int B = pos[2];
        if (R == 6) {
            R = 1;
        }
        heightv = -10000 + ((R * 256 * 256 + G * 256 + B) * 0.1);
        if (heightv < -200) heightv = -200;
    }
    else if (format == 2) {
        uint16_t* data = (uint16_t*)imageData;
        heightv = data[width * y + x];
    }
    return heightv;
}

double OfflineElevation::getHeight(double longitude, double latitude, int level) {
    double u = (longitude - envelope.minX()) / envelope.width();
    double v = 1.0 - (latitude - envelope.minY()) / envelope.height();
    
    double x = u * width;
    double y = v * height;
    int ix = (int)round(x);
    int iy = (int)round(y);
    double heightv = 0;
#if 0
    heightv = sampleHeight(ix, iy);
    return heightv * elevationScale;
#else
    int limitLevel = 11;
    if (level < limitLevel) {
        heightv = sampleHeight(ix, iy);
    }
    else {
        double sumValue = 0;
        double sumWeight = 0;
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                int sx = ix + i;
                int sy = iy + j;
                double dx = (sx - x);
                double dy = (sy - y);
                double disSq = dx * dx + dy * dy;
                if (disSq > 1) continue;

                double value = sampleHeight(sx, sy);
                double weight = (1 - disSq) / disSq;

                sumValue += value * weight;
                sumWeight += weight;
            }
        }
        heightv = sumValue / sumWeight;
    }
#endif
    return heightv * elevationScale;
}

double OfflineElevation::getHeightMercator(double x, double y, int level) {
    Coord2D coor(x, y);
    GeoCoordSys::earth()->fromMercator(&coor);
    return getHeight(coor.x, coor.y, level);
}
