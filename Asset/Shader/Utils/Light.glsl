#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

struct Light
{
    int type;
    float intensity;
    float range;
    float extraData;
    vec3 position;
    vec4 color;
};

#endif