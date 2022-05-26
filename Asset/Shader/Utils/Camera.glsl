#ifndef _CAMERA_GLSL_
#define _CAMERA_GLSL_

#define ORTHOGRAPHIC_CAMERA 1
#define PERSPECTIVE_CAMERA 2

layout(set = 1, binding = 0) uniform CameraData{
    int type;
    float nearFlat;
    float farFlat;
    float aspectRatio;
    vec3 position;
    vec4 parameter;
    vec3 forward;
    vec3 right;
    vec4 clipPlanes[6];
} cameraData;

vec3 OrthographicCameraViewDirection()
{
    return normalize(cameraData.forward);
}

vec3 PerspectiveCameraViewDirection(vec3 viewedPosition)
{
    return normalize(viewedPosition - cameraData.position);
}

vec3 CameraViewDirection(vec3 viewedPosition)
{
    switch(cameraData.type)
    {
        case ORTHOGRAPHIC_CAMERA:
        {
            return OrthographicCameraViewDirection();
        }
        case PERSPECTIVE_CAMERA:
        {
            return PerspectiveCameraViewDirection(viewedPosition);
        }
    }
    return vec3(0, 0, -1);
}

#endif