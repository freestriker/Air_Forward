#pragma once
#include "Core/Object/Object.h"
#include "Utils/ChildBrotherTree.h"
#include <vector>
#include "Core/Component/Component.h"
#include <rttr/type>
#include "Core/Object/LifeCycle.h"
#include "Core/Component/Transform/Transform.h"

namespace Core
{
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
			, public LifeCycle
		{
		private:
			std::vector<Core::Component::Component*> _components;
			Utils::ChildBrotherTree<GameObject> _chain;

			bool OnCheckValid()override;
			void OnAwake()override;
			void OnEnable()override;
			void OnDisable()override;
			void OnDestory()override;

		public:
			std::string name;
			Core::Component::Transform::Transform transform;

			GameObject(std::string name);
			GameObject();
			virtual ~GameObject();

			Core::Component::Component* GetComponent(std::string typeName);
			std::vector<Core::Component::Component*> GetComponents(std::string typeName);
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

			RTTR_ENABLE(Core::Object::Object, Core::Object::LifeCycle)
		};
	}
}