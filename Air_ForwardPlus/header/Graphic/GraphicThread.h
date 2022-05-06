#pragma once
#include "utils/Thread.h"
namespace Graphic
{
	class CommandPool;
	class CommandBuffer;
	class GraphicThread final : public Thread
	{
	public:
		static GraphicThread* const instance;
	private:
		bool _stopped;
		CommandPool* commandPool;
		CommandBuffer* commandBuffer;
	public:
		GraphicThread();
		~GraphicThread();
		void Init()override;
	private:
		void OnStart() override;
		void OnRun() override;
		void OnEnd() override;
	};
}
