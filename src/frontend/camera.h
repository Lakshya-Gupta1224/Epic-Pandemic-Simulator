#ifndef CAMERA_H
#define CAMERA_H

#include "models.h"

void camera_init(CameraState* cam);
void camera_rotate(CameraState* cam, float dAzimuth, float dElevation);
void camera_zoom(CameraState* cam, float delta);
void camera_pan(CameraState* cam, float dx, float dy);
void camera_apply(const CameraState* cam);

#endif
