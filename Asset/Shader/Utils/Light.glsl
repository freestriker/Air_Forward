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

layout(set = 2, binding = 0) uniform samplerCube skyBoxTexture;

layout(set = 3, binding = 0) uniform SkyBox {
    Light light;
}skyBox;
layout(set = 4, binding = 0) uniform MainLight {
    Light light;
}mainLight;

layout(set = 5, binding = 0) uniform ImportantLight{
    Light lights[4];
} importantLight;

layout(set = 6, binding = 0) uniform UnimportantLight{
    Light lights[4];
} unimportantLight;

vec4 EnvironmentLighting(vec3 direction)
{
    vec4 color  = skyBox.light.intensity * skyBox.light.color * texture(skyBoxTexture, direction);
    return vec4(color.xyz, 1);
}

vec4 DiffuseDirectionalLighting(Light light, vec3 normalDirection)
{
    vec4 color = light.intensity * light.color * max(0, dot(normalDirection, -normalize(light.position)));
    return vec4(color.xyz, 1);
}

vec4 SpecularDirectionalLighting(Light light, vec3 viewDirection, vec3 normalDirection, float gloss)
{
    vec3 reflectDirection = normalize(reflect(light.position, normalDirection));
    vec3 inverseViewDirection = normalize(-viewDirection);
    vec4 color = light.intensity * light.color * pow(max(0, dot(reflectDirection, inverseViewDirection)), gloss);
    return vec4(color.xyz, 1);
}

vec4 DiffusePointLighting(Light light, vec3 normalDirection, vec3 illuminatedPosition)
{
    vec3 lightDirection = normalize(illuminatedPosition - light.position);
    float d = distance(light.position, illuminatedPosition);
    float k1 = light.range / max(light.range, d);
    float attenuation = k1 * k1;
    float win = pow(max(1 - pow(d / light.extraData, 4), 0), 2);
    vec4 color = light.intensity * attenuation * win * light.color * max(0, dot(normalDirection, -lightDirection));
    return vec4(color.xyz, 1);
}

vec4 SpecularPointLighting(Light light, vec3 viewDirection, vec3 illuminatedPosition, vec3 normalDirection, float gloss)
{
    vec3 reflectDirection = normalize(reflect(illuminatedPosition - light.position, normalDirection));
    vec3 inverseViewDirection = normalize(-viewDirection);
    vec4 color = light.intensity * light.color * pow(max(0, dot(reflectDirection, inverseViewDirection)), gloss);
    return vec4(color.xyz, 1);
}

vec4 DiffuseLighting(Light light, vec3 normal, vec3 illuminatedPosition)
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
            return DiffusePointLighting(light, normal, illuminatedPosition);
        }
    }
    return vec4(0, 0, 0, 1);
}

vec4 SpecularLighting(Light light, vec3 viewDirection, vec3 illuminatedPosition, vec3 normalDirection, float gloss)
{
    switch(light.type)
    {
        case INVALID_LIGHT:
        {
            return vec4(0, 0, 0, 1);
        }
        case DIRECTIONL_LIGHT:
        {
            return SpecularDirectionalLighting(light, viewDirection, normalDirection, gloss);
        }
        case POINT_LIGHT:
        {
            return SpecularPointLighting(light, viewDirection, illuminatedPosition, normalDirection, gloss);
        }
    }
    return vec4(0, 0, 0, 1);
}

#endif