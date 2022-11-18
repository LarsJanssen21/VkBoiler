#include "MyApp.h"

#include <thread>

void MyApp::PreDeviceSetup(PreDeviceSetupParameters& params)
{
	/*	Example Code	*/
	{
		params.WindowWidth = 1280;
		params.WindowHeight = 720;

		params.EnableDeviceDebugging = true;
		params.DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		QueueType type{};
		type.Types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		type.Count = 2;
		params.DesiredQueues.push_back(type);
	}
}

void MyApp::Init()
{
	VkQueue queueHandles[2] = {};
	uint32_t handleIndex = 0;
	for (const auto& indices : m_QueueIndices)
	{
		for (size_t i = 0; i < indices.FamilyCount; i++)
		{
			for (uint32_t offset = 0; offset < indices.Count[i]; offset++)
			{
				vkGetDeviceQueue(m_Device, indices.Families[i], indices.FirstIndex[i] + offset, &queueHandles[handleIndex]);
				handleIndex++;
			}
		}
	}
}

void MyApp::Tick()
{

}

void MyApp::Destroy()
{

}