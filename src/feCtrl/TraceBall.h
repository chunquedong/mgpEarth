/*
 * Copyright (c) 2023, chunquedong/yangjiandong
 *
 * This file is part of fastEarth project
 * all rights reserved
 *
 */
#if 0
#ifndef TRACEBALL_H
#define TRACEBALL_H

#include "feCtrl/EarthCtrl.h"

FE_BEGIN_NAMESPACE

class TraceBall : public EarthCtrl {
  Quaternion earthRotation;
public:
  TraceBall();

  virtual Coord2D getPosition() override;

  /**
   * set look at surface postion in geo coordinate system
   */
  virtual void moveToPostion(Coord2D pos) override;

  virtual void setRotationZ(double rotZ) override;

  virtual void moveByPixel(float dx, float dy) override;

protected:
  virtual void updateCameraTransform(Camera &camera, Rectangle &viewport) override;

private:
  /**
   * project screen center near point to earth shpere surface point
   */
  void projectToShpere(float x, float y, Vector &vec);

  /**
   * convert geo coordinate system to Quaternion rotation
   */
  void blToQuaternion(Coord2D &postion, Quaternion &out);

  Coord2D quaternionToBl(Quaternion &quat);
};

FE_END_NAMESPACE
#endif // TRACEBALL_H
#endif