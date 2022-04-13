#pragma once
#include "utils/Thread.h"
namespace Graphic
{
	class GraphicThread final : public Thread
	{
	public:
	private:
		bool _stopped;
	public:
		GraphicThread();
		~GraphicThread();
	private:
		void OnStart() override;
		void OnRun() override;
		void OnEnd() override;
	};
}
