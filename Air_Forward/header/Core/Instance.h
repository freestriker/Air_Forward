#pragma once
#include <Core/Object/GameObject.h>

namespace Core
{
	namespace Object
	{
		class GameObject;
	}
	class Thread;
	class Instance final
	{
		friend class Core::Thread;
	public:
		class RootGameObject final
		{
			friend class Core::Instance;
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
	private:
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
