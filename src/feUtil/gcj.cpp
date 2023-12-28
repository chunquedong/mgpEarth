
#include <math.h>
#include "gcj.h"

/*
 pi: 圆周率。
 a: 卫星椭球坐标投影到平面地图坐标系的投影因子。
 ee: 椭球的偏心率。
 x_pi: 圆周率转换量。
*/
 
double pi = 3.14159265358979324;		
double a = 6378245.0;
double ee = 0.00669342162296594323;
double x_pi = 3.14159265358979324 * 3000.0 / 180.0;
 
int wgs2bd(double lat, double lon, double* pLat, double* pLon); // WGS84=>BD09 地球坐标系=>百度坐标系 
int gcj2bd(double lat, double lon, double* pLat, double* pLon);	// GCJ02=>BD09 火星坐标系=>百度坐标系 
int bd2gcj(double lat, double lon, double* pLat, double* pLon);	// BD09=>GCJ02 百度坐标系=>火星坐标系 
int wgs2gcj(double lat, double lon, double* pLat, double* pLon);// WGS84=>GCJ02 地球坐标系=>火星坐标系
int gcj2wgs(double lat, double lon, double* pLat, double* pLon);// GCJ02=>WGS84 火星坐标系=>地球坐标系(粗略)
int bd2wgs(double lat, double lon, double* pLat, double* pLon);	// BD09=>WGS84 百度坐标系=>地球坐标系(粗略)
int gcj2wgs_Exactly(double lat, double lon, double* wgs_Lat, double* wgs_lon);// GCJ02=>WGS84 火星坐标系=>地球坐标系（精确）
int bd2wgs_Exactly(double lat, double lon, double* pLat, double* pLon);// BD09=>WGS84 百度坐标系=>地球坐标系(精确)
double *OffSet(double lat, double lon);	// 偏移量
double transformLat(double x, double y);// 纬度偏移量
double transformLon(double x, double y);// 经度偏移量
int outOfChina(double lat, double lon);
 
// WGS84=>BD09 地球坐标系=>百度坐标系 
int wgs2bd(double lat, double lon, double* pLat, double* pLon) {
   double lat_ = 0.0, lon_ = 0.0;
   wgs2gcj(lat, lon, &lat_, &lon_);
   gcj2bd(lat_, lon_,  pLat, pLon);
   return 0;
}
 
// GCJ02=>BD09 火星坐标系=>百度坐标系  
int gcj2bd(double lat, double lon, double* pLat, double* pLon) {
   double x = lon, y = lat;
   double z = sqrt(x * x + y * y) + 0.00002 * sin(y * x_pi);
   double theta = atan2(y, x) + 0.000003 * cos(x * x_pi);
   *pLon = z * cos(theta) + 0.0065;
   *pLat = z * sin(theta) + 0.006;
   return 0;
}
 
// BD09=>GCJ02 百度坐标系=>火星坐标系 
int bd2gcj(double lat, double lon, double* pLat, double* pLon) {
   double x = lon - 0.0065, y = lat - 0.006;
   double z = sqrt(x * x + y * y) - 0.00002 * sin(y * x_pi);
   double theta = atan2(y, x) - 0.000003 * cos(x * x_pi);
   *pLon = z * cos(theta);
   *pLat = z * sin(theta);
   return 0;
}
 
// WGS84=>GCJ02 地球坐标系=>火星坐标系
int wgs2gcj(double lat, double lon, double* pLat, double* pLon) {
    if (outOfChina(lat,lon))
    {
        *pLat = lat;
        *pLon = lon;
        return 0;
    }
   double dLat = transformLat(lon - 105.0, lat - 35.0);
   double dLon = transformLon(lon - 105.0, lat - 35.0);
   double radLat = lat / 180.0 * pi;
   double magic = sin(radLat);
   magic = 1 - ee * magic * magic;
   double sqrtMagic = sqrt(magic);
   dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
   dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
   *pLat = lat + dLat;
   *pLon = lon + dLon;
   return 0;
}
 
// GCJ02=>WGS84 火星坐标系=>地球坐标系(粗略)
int gcj2wgs(double lat, double lon, double* pLat, double* pLon) {
    if (outOfChina(lat,lon))
    {
        *pLat = lat;
        *pLon = lon;
        return 0;
    }
	double *offset;
	offset = OffSet(lat,lon);
	*pLat = lat - offset[0];
 	*pLon = lon - offset[1];
	return 0;
}
 
// GCJ02=>WGS84 火星坐标系=>地球坐标系（精确）
int gcj2wgs_Exactly(double gcjlat, double gcjlon, double* wgs_Lat, double* wgs_lon) {
    if (outOfChina(gcjlat,gcjlon))
    {
        *wgs_Lat = gcjlat;
        *wgs_lon = gcjlon;
        return 0;
    }
	double initDelta = 0.01;
	double threshold = 0.000000001;
	double dLat = initDelta, dLon = initDelta;
	double mLat = gcjlat - dLat, mLon = gcjlon - dLon;
	double pLat = gcjlat + dLat, pLon = gcjlon + dLon;
	double wgsLat = 0.0, wgslon = 0.0, i = 0.0 ,newgcjlat = 0.0,newgcjlon = 0.0;
 
	while (true) {
		wgsLat = (mLat + pLat) / 2;
		wgslon = (mLon + pLon) / 2;		
		wgs2gcj(wgsLat,wgslon,&newgcjlat,&newgcjlon);
		dLon = newgcjlon - gcjlon;
		dLat = newgcjlat - gcjlat;
		if ((fabs(dLat) < threshold) && (fabs(dLon) < threshold))
			break;
							
		if (dLat > 0)
			pLat = wgsLat;
		else
			mLat = wgsLat;
		if (dLon > 0)
			pLon = wgslon;
		else
			mLon = wgslon;
 
		if (++i > 10000)
			break;
	}
	*wgs_Lat = wgsLat;
	*wgs_lon = wgslon;
	return 0;
}
 
// BD09=>WGS84 百度坐标系=>地球坐标系(粗略)
int bd2wgs(double lat, double lon, double* pLat, double* pLon) {
	double lat_ = 0.0, lon_ = 0.0;
 	bd2gcj(lat, lon, &lat_, &lon_);
 	gcj2wgs(lat_, lon_,  pLat, pLon);
 	return 0;
}

// BD09=>WGS84 百度坐标系=>地球坐标系(精确)
int bd2wgs_Exactly(double lat, double lon, double* pLat, double* pLon) {
	double lat_ = 0.0, lon_ = 0.0;
 	bd2gcj(lat, lon, &lat_, &lon_);
 	gcj2wgs_Exactly(lat_, lon_,  pLat, pLon);
 	return 0;
}

// 偏移量
double *OffSet(double lat, double lon) {
    double Latlon[2] = {0.0,0.0};
    double dLat = transformLat(lon - 105.0, lat - 35.0);
    double dLon = transformLon(lon - 105.0, lat - 35.0);
    double radLat = lat / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    Latlon[0] = dLat;
    Latlon[1] = dLon;
    return Latlon;
}
 
// 纬度偏移量
double transformLat(double x, double y) {
       double ret = 0.0;
       ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(fabs(x));
       ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
       ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
       ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi  / 30.0)) * 2.0 / 3.0;
       return ret;
}
 
// 经度偏移量
double transformLon(double x, double y) {
       double ret = 0.0;
       ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(fabs(x));
       ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
       ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
       ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;
       return ret;
}

/**
 * Description: 中国境外返回true,境内返回false
 * @param lon 	经度
 * @param lat	纬度
 * @return
 */
int outOfChina(double lat, double lon) {
    if (lon < 72.004 || lon > 137.8347)
        return true;
    if (lat < 0.8293 || lat > 55.8271)
        return true;
    return false;
}
