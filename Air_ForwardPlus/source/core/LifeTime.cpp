#include "core/LifeTime.h"
#include <rttr/registration>
RTTR_REGISTRATION
{
    using namespace rttr;
    registration::class_<LifeTime>("LifeTime")
        .constructor<>()
        .method("OnAwake", &LifeTime::OnAwake)
        .method("OnStart", &LifeTime::OnStart)
        .method("OnUpdate", &LifeTime::OnUpdate)
        .method("OnDestory", &LifeTime::OnDestory)
        .method("OnEnable", &LifeTime::OnEnable)
        .method("OnDisable", &LifeTime::OnDisable)
        ;
}

LifeTime::LifeTime()
{
}

LifeTime::~LifeTime()
{
}

void LifeTime::OnAwake()
{
}

void LifeTime::OnStart()
{
}

void LifeTime::OnUpdate()
{
}

void LifeTime::OnDestory()
{
}

void LifeTime::OnEnable()
{
}

void LifeTime::OnDisable()
{
}
