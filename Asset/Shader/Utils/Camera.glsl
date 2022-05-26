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

#endif