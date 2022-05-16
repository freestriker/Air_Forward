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
			static Graphic::Command::CommandPool* _transferCommandPool;
			static Graphic::Command::CommandBuffer* _transferCommandBuffer;
		public:
			static IO::Manager::AssetManager assetManager;
		public:
		};
	}
}