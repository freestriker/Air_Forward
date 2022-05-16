#pragma once
#include "Core/Object/Object.h"
#include "Core/Object/LifeCycle.h"
#include "Utils/ActivableBase.h"
namespace Core
{
	namespace Manager
	{
		class ObjectFactory;
	}
	namespace Object
	{
		class GameObject;
	}
	namespace Component
	{
		class Component 
			: public Core::Object::Object
			, public Utils::ActivableBase
			, public Core::Object::LifeCycle
		{
			friend class Core::Object::GameObject;
			friend class Manager::ObjectFactory;
		protected:
			Core::Object::GameObject* _gameObject;
			Component();
			virtual ~Component();
		public:
			Core::Object::GameObject* GameObject();

			RTTR_ENABLE(Core::Object::Object, Utils::ActivableBase)
		};
	}
}
