#pragma once

#include <string>
#include <optional>

#include <vulkan/vulkan.h>

#include "Window/Window.h"

#include <vector>

#undef CreateWindow

struct QueueType
{
	VkQueueFlags Types;
	uint32_t Count;
};

struct QueueIndices
{
	VkQueueFlags Types;
	uint32_t FamilyCount;
	std::vector<uint32_t> Families;
	std::vector<uint32_t> FirstIndex;
	std::vector<uint32_t> Count;
};

struct PreDeviceSetupParameters
{
	std::string AppName = "";
	uint32_t WindowWidth = 640, WindowHeight = 640;
	bool AllowWindowResizing = false;
	bool EnableDeviceDebugging = false;

	std::vector<const char*> ValidationLayers = {};
	std::vector<const char*> InstanceExtensions = {};
	std::vector<const char*> DeviceExtensions = {};
	VkPhysicalDeviceFeatures EnabledDeviceFeatures = {};

	std::vector<QueueType> DesiredQueues = {};
};

class VulkanApp
{
	friend class AppHandler;
public:
	VulkanApp() { }
	~VulkanApp() = default;


	/// <summary>
	/// Enabling Features
	/// </summary>
	virtual void PreDeviceSetup(PreDeviceSetupParameters& params) { }
	/// <summary>
	/// Gets called after creation of base app vulkan parameters
	/// </summary>
	virtual void Init() { }
	/// <summary>
	/// Runs each frame
	/// </summary>
	virtual void Tick() { }
	/// <summary>
	/// Gets called before destruction of base app parameters
	/// </summary>
	virtual void Destroy() { }

private:
	/// <summary>
	/// Initializaes Vulkan and window
	/// </summary>
	void BaseInit();
	/// <summary>
	/// Destroys vulkan context and window
	/// </summary>
	void BaseDestroy();
	/// <summary>
	/// Update window
	/// </summary>
	void WindowUpdate();

	bool CreateInstance(const PreDeviceSetupParameters& params);
	bool CreateWindow(const PreDeviceSetupParameters& params);
	bool CreateSurface();
	bool PickPhysicalDevice(const PreDeviceSetupParameters& params);
	bool QueryPhysicalDeviceQueues(VkPhysicalDevice device, 
		const PreDeviceSetupParameters& params,
		std::vector<QueueIndices>* pQueueIndices = nullptr, 
		std::vector<VkDeviceQueueCreateInfo>* pQueueCis = nullptr);
	bool CreateLogicalDevice(const PreDeviceSetupParameters& params);

protected:
	/// <summary>
	/// This variable defines whether the application should keep running.
	/// This must be considered as the only valid method of destroying the app
	/// </summary>
	bool m_Running = true;

	Window* m_pWindow = nullptr;

	VkInstance m_Instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_PhysDevice = VK_NULL_HANDLE;
	std::vector<QueueIndices> m_QueueIndices = {};
	uint32_t m_TotalQueueCount = 0;
	VkDevice m_Device = VK_NULL_HANDLE;
};