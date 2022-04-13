#include "Graphic/GraphicThread.h"
#include "Graphic/Creator/GlfwWindowCreator.h"
#include "Graphic/Creator/VulkanInstanceCreator.h"
#include "Graphic/Creator/VulkanDeviceCreator.h"
#include "Graphic/GlobalInstance.h"

Graphic::GraphicThread::GraphicThread()
	: _stopped(true)
{

}

Graphic::GraphicThread::~GraphicThread()
{

}

void Graphic::GraphicThread::OnStart()
{
	Graphic::GlfwWindowCreator glfwWindowCreator = Graphic::GlfwWindowCreator();
	Graphic::GlobalInstance::CreateGlfwWindow(&glfwWindowCreator);
	Graphic::VulkanInstanceCreator vulkanInstanceCreator = Graphic::VulkanInstanceCreator();
	Graphic::GlobalInstance::CreateVulkanInstance(&vulkanInstanceCreator);
	Graphic::VulkanDeviceCreator vulkanDeviceCreator = Graphic::VulkanDeviceCreator();
	Graphic::GlobalInstance::CreateVulkanDevice(&vulkanDeviceCreator);

	_stopped = false;
}

void Graphic::GraphicThread::OnRun()
{
	while (!_stopped)
	{

	}
}

void Graphic::GraphicThread::OnEnd()
{
	_stopped = true;
}
