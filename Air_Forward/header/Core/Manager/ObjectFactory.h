#pragma once

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
	namespace Manager
	{
		class ObjectFactory final
		{
		private:
			ObjectFactory();
			~ObjectFactory();
		public:
			static void Destroy(Component::Component* component);
			static void Destroy(Object::GameObject* gameObject);
		};
	}
}