#include "camera.h"
#include <math.h>
#include <GL/glut.h>

void camera_init(CameraState* cam) {
    cam->distance    = CAM_DEFAULT_DIST;
    cam->azimuth     = CAM_DEFAULT_AZIMUTH;
    cam->elevation   = CAM_DEFAULT_ELEVATION;
    cam->target.x    = 0.0f;
    cam->target.y    = 0.0f;
    cam->target.z    = 0.0f;
    cam->panSpeed    = 0.8f;
    cam->rotateSpeed = 0.3f;
    cam->zoomSpeed   = 15.0f;
    cam->minDist     = CAM_MIN_DIST;
    cam->maxDist     = CAM_MAX_DIST;
    cam->minElev     = CAM_MIN_ELEV;
    cam->maxElev     = CAM_MAX_ELEV;
}

void camera_rotate(CameraState* cam, float dAzimuth, float dElevation) {
    cam->azimuth   += dAzimuth * cam->rotateSpeed;
    cam->elevation += dElevation * cam->rotateSpeed;
    if (cam->elevation < cam->minElev) cam->elevation = cam->minElev;
    if (cam->elevation > cam->maxElev) cam->elevation = cam->maxElev;
}

void camera_zoom(CameraState* cam, float delta) {
    cam->distance -= delta * cam->zoomSpeed;
    if (cam->distance < cam->minDist) cam->distance = cam->minDist;
    if (cam->distance > cam->maxDist) cam->distance = cam->maxDist;
}

void camera_pan(CameraState* cam, float dx, float dy) {
    float rad = (float)(cam->azimuth * DEG_TO_RAD);
    cam->target.x += (-dx * cosf(rad) - dy * sinf(rad)) * cam->panSpeed;
    cam->target.z += ( dx * sinf(rad) - dy * cosf(rad)) * cam->panSpeed;
}

void camera_apply(const CameraState* cam) {
    float azRad  = (float)(cam->azimuth  * DEG_TO_RAD);
    float elRad  = (float)(cam->elevation * DEG_TO_RAD);

    float eyeX = cam->target.x + cam->distance * cosf(elRad) * sinf(azRad);
    float eyeY = cam->target.y + cam->distance * sinf(elRad);
    float eyeZ = cam->target.z + cam->distance * cosf(elRad) * cosf(azRad);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ,
              cam->target.x, cam->target.y, cam->target.z,
              0.0, 1.0, 0.0);
}
