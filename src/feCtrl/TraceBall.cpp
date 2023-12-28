#if 0
#include "feCtrl/TraceBall.h"

#include "mgp.h"
#include "feTile/TileLayer.h"
#include "feModel/GeoCoordSys.h"


FE_USING_NAMESPACE

TraceBall::TraceBall()
{
  earthRotation.x = 0;
  earthRotation.y = 0;
  earthRotation.z = 0;
  earthRotation.w = 1;
}

Coord2D TraceBall::getPosition()
{
  return quaternionToBl(earthRotation);
}

void TraceBall::moveToPostion(Coord2D pos)
{
  blToQuaternion(pos, earthRotation);
  invalidateCamera();
}

void TraceBall::setRotationZ(double rotZ)
{
  double delta = rotZ - this->getRotationZ();
  EarthCtrl::setRotationZ(rotZ);

  Quaternion que1;
  Vector v1;
  v1.set(0, 0, 1);
  que1.fromAxis(&v1, Math::toRadians(delta));

  que1.mult(&earthRotation);
  earthRotation = que1;

  invalidateCamera();
}

void TraceBall::updateCameraTransform(Camera &camera, Rectangle &viewport)
{
  CF_UNUSED(viewport);
  //translate
  camera.transform.makeIndentity(4);
  Transform3D::translate(&camera.transform, 0, 0, -distanceToCenter);
  Transform3D::rotate(&camera.transform, rotationX, 1, 0, 0);

  //rotate
  Matrix rotate;
  earthRotation.getMatrix(&rotate);

  Matrix out;
  camera.transform.mult(&rotate, &out);
  camera.transform = out;
}

void TraceBall::moveByPixel(float dx, float dy)
{
  if (dx == 0 && dy == 0) {
    return;
  }

  //projection
  Vector v1, v2, rotAxis;
  v1.set(0, 0, GeoCoordSys::earth()->getRadius());
  projectToShpere(dx, -dy, v2);

  //get rotate axis
  v1.crossProduct3(&v2, &rotAxis);
  rotAxis.normalize();

  //get angle
  v1.sub(&v2);
  double dis = v1.length() / GeoCoordSys::earth()->getRadius();
  double angle = dis;//asin(dis/2)*2;
  Quaternion quat;

  //make quaternion
  quat.fromAxis(&rotAxis, angle);

  //apply to rotation
  quat.mult(&earthRotation);
  earthRotation = quat;
  invalidateCamera();
}

void TraceBall::blToQuaternion(Coord2D &postion, Quaternion &out) {
  float y = Math::clamp(postion.y, -90.0, 90.0);
  float x = Math::clamp(postion.x, -180.0, 180.0);

  Vector ya;
  ya.set(0, 1, 0);
  Vector za;
  za.set(0, 0, 1);
  Vector xa;
  xa.set(1, 0, 0);

  Quaternion qua1;
  qua1.fromAxis(&xa, Math::toRadians(-90));
  Quaternion qua2;
  qua2.fromAxis(&ya, Math::toRadians(-90));
  qua2.mult(&qua1);

  Quaternion qua3;
  qua3.fromAxis(&za, Math::toRadians(-x));
  Quaternion qua4;
  qua4.fromAxis(&ya, Math::toRadians(y));
  qua4.mult(&qua3);

  out = qua2;
  out.mult(&qua4);
}

Coord2D TraceBall::quaternionToBl(Quaternion &quat)
{
  Vector v1;
  v1.set(0, 0, GeoCoordSys::earth()->getRadius());
  Matrix rotate, invertRotate;
  quat.getMatrix(&rotate);
  rotate.invertByAdjoint(&invertRotate);

  invertRotate.multVector(v1);
  Coord2D pos;
  GeoCoordSys::xyzToBl(v1, pos);
  return pos;
}

void TraceBall::projectToShpere(float x, float y, Vector &vec)
{
  double s = xyScale();
  double dx = x / s;
  double dy = y / s;
  vec.x = dx;
  vec.y = dy;
  //vec.w() = 1;

  double proLength2 = (dx*dx) + (dy*dy);
  double z2 = (GeoCoordSys::earth()->getRadius()*GeoCoordSys::earth()->getRadius()) - proLength2;
  vec.z = z2 < 0 ? 0 : sqrt(z2);
}
#endif