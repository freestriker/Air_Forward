#pragma once
#include <Logic/Object/GameObject.h>
#include <unordered_set>
#include <Utils/Condition.h>
#include "Utils/Time.h"

namespace Logic
{
	namespace Object
	{
		class GameObject;
	}
	namespace Component
	{
		class Component;
	}
	namespace Core
	{
		class Thread;
		class Instance final
		{
			friend class Core::Thread;
			friend class Logic::Object::GameObject;
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
			class Time
			{
				friend class Core::Instance;
				friend class Core::Thread;
			private:
				Utils::Time _time;
				Time();
				~Time();
				inline void Launch();
				inline void Refresh();
			public:
				inline double DeltaDuration();
				inline double LaunchDuration();
			};
			static RootGameObject rootObject;
			static Time time;
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
}

inline void Logic::Core::Instance::RootGameObject::AddChild(Object::GameObject* child)
{
	_gameObject.AddChild(child);
}

inline void Logic::Core::Instance::RootGameObject::RemoveChild(Object::GameObject* child)
{
	_gameObject.RemoveChild(child);
}

inline Logic::Object::GameObject* Logic::Core::Instance::RootGameObject::Child()
{
	return _gameObject.Child();
}
inline void Logic::Core::Instance::Time::Launch()
{
	_time.Launch();
}
inline void Logic::Core::Instance::Time::Refresh()
{
	_time.Refresh();
}
inline double Logic::Core::Instance::Time::DeltaDuration()
{
	return _time.DeltaDuration();
}
inline double Logic::Core::Instance::Time::LaunchDuration()
{
	return _time.LaunchDuration();
}
