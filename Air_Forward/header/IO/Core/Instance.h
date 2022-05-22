#pragma once
#include "IO/Manager/AssetManager.h"

namespace Graphic
{
	namespace Command
	{
		class CommandPool;
		class CommandBuffer;
	}
}

namespace IO
{
	namespace Core
	{
		class Thread;
		class Instance final
		{
			friend class Thread;
		private:
			Instance();
			~Instance();
		public:
			static IO::Manager::AssetManager assetManager;
		public:
		};
	}
}