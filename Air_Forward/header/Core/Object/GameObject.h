#pragma once
#include "Core/Object/Object.h"
#include "Utils/ChildBrotherTree.h"
#include <vector>
#include <list>
#include "Core/Component/Component.h"
#include <rttr/type>
#include "Core/Component/Transform/Transform.h"
#include "Utils/ActivableBase.h"
#include "Utils/CrossLinkableNode.h"

namespace Core
{
	class Thread;
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
			friend class Core::Thread;
		private:
			Utils::CrossLinkableColHead _timeSqueueComponentsHead;
			std::map<Component::Component::ComponentType, std::unique_ptr< Utils::CrossLinkableRowHead>> _typeSqueueComponentsHeadMap;

			Utils::ChildBrotherTree<GameObject> _chain;

			GameObject(const GameObject&) = delete;
			GameObject& operator=(const GameObject&) = delete;
			GameObject(GameObject&&) = delete;
			GameObject& operator=(GameObject&&) = delete;
		public:
			void _RemoveComponent1(Core::Component::Component* component);
			Core::Component::Component* _RemoveComponent2(std::string typeName);
			void _RemoveComponents1(std::vector<Component::Component*> components);
			std::vector<Core::Component::Component*> _RemoveComponents2(std::string typeName);
			Core::Component::Component* _GetComponent(std::string targetTypeName);
			std::vector <Core::Component::Component*> _GetComponents(std::string targetTypeName);

		public:
			std::string name;
			Core::Component::Transform::Transform transform;

			GameObject(std::string name);
			GameObject();
			virtual ~GameObject();

			//Add
			void AddComponent(Core::Component::Component* component);
			//Remove
			void RemoveComponent(Core::Component::Component* component);
			Core::Component::Component* RemoveComponent(std::string typeName);
			Core::Component::Component* RemoveComponent(rttr::type targetType);
			template<typename TType>
			TType* RemoveComponent();
			void RemoveComponents(std::vector<Component::Component*> components);
			std::vector<Core::Component::Component*> RemoveComponents(std::string typeName);
			std::vector<Core::Component::Component*> RemoveComponents(rttr::type targetType);
			template<typename TType>
			std::vector <TType*> RemoveComponents();
			//Get
			Core::Component::Component* GetComponent(rttr::type targetType);
			Core::Component::Component* GetComponent(std::string targetTypeName);
			template<typename TType>
			TType* GetComponent();
			std::vector<Core::Component::Component*> GetComponents(rttr::type targetType);
			std::vector <Core::Component::Component*> GetComponents(std::string targetTypeName);
			template<typename TType>
			std::vector <TType*> GetComponents();

			bool HaveParent();
			bool HaveChild();
			bool HaveBrother();

			GameObject* Parent();
			GameObject* Child();
			GameObject* Brother();

			void AddChild(GameObject* child);
			void RemoveChild(GameObject* child);
			void RemoveSelf();

			RTTR_ENABLE(Core::Object::Object, Utils::ActivableBase)
		};

		template<typename TType>
		inline TType* GameObject::RemoveComponent()
		{
			return dynamic_cast<TType*>(RemoveComponent(rttr::type::get<TType>()));
		}
		template<typename TType>
		inline std::vector<TType*> GameObject::RemoveComponents()
		{
			auto foundComponents = RemoveComponents(rttr::type::get<TType>());
			auto targetComponents = std::vector<TType*>(foundComponents.size());

			for (uint32_t i = 0; i < targetComponents.size(); i++)
			{
				targetComponents[i] = dynamic_cast<TType*>(foundComponents[i]);
			}

			return targetComponents;
		}
		template<typename TType>
		inline TType* GameObject::GetComponent()
		{
			return dynamic_cast<TType*>(GetComponent(rttr::type::get<TType>()));
		}
		template<typename TType>
		inline std::vector<TType*> GameObject::GetComponents()
		{
			auto foundComponents = GetComponents(rttr::type::get<TType>());
			auto targetComponents = std::vector<TType*>(foundComponents.size());

			for (uint32_t i = 0; i < targetComponents.size(); i++)
			{
				targetComponents[i] = dynamic_cast<TType*>(foundComponents[i]);
			}

			return targetComponents;
		}
	}
}