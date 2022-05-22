#pragma once
#include "Core/Object/Object.h"
#include "Core/Object/LifeCycle.h"
#include "Utils/ActivableBase.h"
#include "Utils/CrossLinkableNode.h"
namespace Core
{
	class Thread;
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
			, private Utils::CrossLinkableNode
		{
			friend class Core::Object::GameObject;
			friend class Manager::ObjectFactory;
			friend class Core::Thread;
		public:
			enum class ComponentType
			{
				DEFAULT,
				TRANSFORM,
				BEHAVIOUR,
				CAMERA
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
			Core::Object::GameObject* _gameObject;
			Component();
			Component(ComponentType type);
			virtual ~Component();
		public:
			Core::Object::GameObject* GameObject();
			ComponentType GetComponentType();
			RTTR_ENABLE(Core::Object::Object, Utils::ActivableBase)
		};
	}
}
