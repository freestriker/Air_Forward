#pragma once
#include <Core/Object/GameObject.h>
#include <unordered_set>
#include <Utils/Condition.h>

namespace Core
{
	namespace Object
	{
		class GameObject;
	}
	namespace Component
	{
		class Component;
	}
	class Thread;
	class Instance final
	{
		friend class Core::Thread;
		friend class Core::Object::GameObject;
	public:
		class RootGameObject final
		{
			friend class Core::Instance;
			friend class Core::Thread;
		private:
			Object::GameObject _gameObject;
			RootGameObject();
			~RootGameObject();
		public:
			inline void AddChild(Object::GameObject* child);
			inline void RemoveChild(Object::GameObject* child);
			inline Object::GameObject* Child();

		};
		static RootGameObject rootObject;
		static void Exit();
		static void WaitExit();
	private:
		static Utils::Condition* _exitCondition;
		static std::unordered_set< Object::GameObject*> _validGameObjectInIteration;
		static std::unordered_set< Component::Component*> _validComponentInIteration;
		Instance();
		~Instance();
	};
}

inline void Core::Instance::RootGameObject::AddChild(Object::GameObject* child)
{
	_gameObject.AddChild(child);
}

inline void Core::Instance::RootGameObject::RemoveChild(Object::GameObject* child)
{
	_gameObject.RemoveChild(child);
}

inline Core::Object::GameObject* Core::Instance::RootGameObject::Child()
{
	return _gameObject.Child();
}
