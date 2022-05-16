#include "IO/Core/Instance.h"

IO::Manager::AssetManager IO::Core::Instance::assetManager = IO::Manager::AssetManager();
Graphic::Command::CommandPool* IO::Core::Instance::_transferCommandPool = nullptr;
Graphic::Command::CommandBuffer* IO::Core::Instance::_transferCommandBuffer = nullptr;

IO::Core::Instance::Instance()
{

}

IO::Core::Instance::~Instance()
{

}
