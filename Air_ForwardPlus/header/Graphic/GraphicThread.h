#pragma once
#include "utils/Thread.h"
namespace Graphic
{
	class GraphicThread final : public Thread
	{
	public:
		static GraphicThread* const instance;
	private:
		bool _stopped;
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
