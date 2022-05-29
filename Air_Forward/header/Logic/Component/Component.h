#pragma once
#include "Logic/Object/Object.h"
#include "Logic/Object/LifeCycle.h"
#include "Utils/ActivableBase.h"
#include "Utils/CrossLinkableNode.h"
namespace Logic
{
	namespace Core
	{
		class Thread;
	}
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
			: public Object::Object
			, public Utils::ActivableBase
			, public Object::LifeCycle
			, private Utils::CrossLinkableNode
		{
			friend class Logic::Object::GameObject;
			friend class Manager::ObjectFactory;
			friend class Core::Thread;
		public:
			enum class ComponentType
			{
				DEFAULT,
				TRANSFORM,
				BEHAVIOUR,
				CAMERA,
				RENDERER,
				LIGHT
			};
		private:
			Component(const Component&) = delete;
			Component& operator=(const Component&) = delete;
			Component(Component&&) = delete;
			Component& operator=(Component&&) = delete;
		protected:
			const static std::map<rttr::type, ComponentType> TYPE_MAP;
			const static rttr::type COMPONENT_TYPE;
			ComponentType _type;
			Logic::Object::GameObject* _gameObject;
			Component();
			Component(ComponentType type);
			virtual ~Component();
		public:
			Logic::Object::GameObject* GameObject();
			ComponentType GetComponentType();
			RTTR_ENABLE(Logic::Object::Object, Utils::ActivableBase)
		};
	}
}