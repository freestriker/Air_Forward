#pragma once
#include "Core/Object/Object.h"
#include "Utils/ChildBrotherTree.h"
#include <vector>
#include <list>
#include "Core/Component/Component.h"
#include <rttr/type>
#include "Core/Component/Transform/Transform.h"
#include "Utils/ActivableBase.h"

namespace Core
{
	namespace Manager
	{
		class ObjectFactory;
	}
	namespace Component
	{
		class Component;
		namespace Transform
		{
			class Transform;
		}
	}
	namespace Object
	{
		class GameObject final 
			: public Object
			, public Utils::ActivableBase
		{
			friend class Manager::ObjectFactory;
		private:
			std::list<Core::Component::Component*> _components;
			Utils::ChildBrotherTree<GameObject> _chain;

			GameObject(const GameObject&) = delete;
			GameObject& operator=(const GameObject&) = delete;
			GameObject(GameObject&&) = delete;
			GameObject& operator=(GameObject&&) = delete;

		public:
			std::string name;
			Core::Component::Transform::Transform transform;

			GameObject(std::string name);
			GameObject();
			virtual ~GameObject();

			Core::Component::Component* GetComponent(std::string typeName);
			std::vector<Core::Component::Component*> GetComponents(std::string typeName);
			bool HaveParent();
			bool HaveChild();
			bool HaveBrother();
			void AddComponent(Core::Component::Component* component);
			void RemoveComponent(Core::Component::Component* component);
			Core::Component::Component* RemoveComponent(std::string typeName);
			std::vector<Core::Component::Component*> RemoveComponents(std::string typeName);

			GameObject* Parent();
			GameObject* Child();
			GameObject* Brother();

			void AddChild(GameObject* child);
			void RemoveChild(GameObject* child);
			void RemoveSelf();

			RTTR_ENABLE(Core::Object::Object, Utils::ActivableBase)
		};
	}
}