#ifndef _LIGHT_GLSL_
#define _LIGHT_GLSL_

#define INVALID_LIGHT 0
#define DIRECTIONL_LIGHT 1
#define POINT_LIGHT 2

struct Light
{
    int type;
    float intensity;
    float range;
    float extraData;
    vec3 position;
    vec4 color;
};

vec4 DiffuseDirectionalLighting(Light light, vec3 normal)
{
    vec4 color = light.intensity * light.color * max(0, dot(normal, -normalize(light.position)));
    return vec4(color.xyz, 1);
}

vec4 DiffusePointLighting(Light light, vec3 normal, vec3 lightingPoint)
{
    vec3 lightDir = normalize(lightingPoint - light.position);
    float d = distance(light.position, lightingPoint);
    float k1 = light.range / max(light.range, d);
    float attenuation = k1 * k1;
    float win = pow(max(1 - pow(d / light.extraData, 4), 0), 2);
    vec4 color = light.intensity * attenuation * win * light.color * max(0, dot(normal, -lightDir));
    return vec4(color.xyz, 1);
}

vec4 DiffuseLighting(Light light, vec3 normal, vec3 lightingPoint)
{
    switch(light.type)
    {
        case INVALID_LIGHT:
        {
            return vec4(0, 0, 0, 1);
        }
        case DIRECTIONL_LIGHT:
        {
            return DiffuseDirectionalLighting(light, normal);
        }
        case POINT_LIGHT:
        {
            return DiffusePointLighting(light, normal, lightingPoint);
        }
    }
    return vec4(0, 0, 0, 1);
}

#endif