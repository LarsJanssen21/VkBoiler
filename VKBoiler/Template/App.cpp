#include "App.h"

#define OUT_CODE(condition) if(!condition) { m_Running = false; return; }

#include <vulkan/vulkan_win32.h>
#include <vector>

void VulkanApp::BaseInit()
{
	// Setting up parameters pre device creation
	PreDeviceSetupParameters params = {};
	PreDeviceSetup(params);

	// Own init code
	OUT_CODE(CreateInstance(params))
	OUT_CODE(CreateWindow(params));
	OUT_CODE(CreateSurface());
	OUT_CODE(PickPhysicalDevice(params));
	OUT_CODE(CreateLogicalDevice(params));
}

void VulkanApp::BaseDestroy()
{
	// Own destroy code
	if(m_Device != VK_NULL_HANDLE)
		vkDestroyDevice(m_Device, nullptr);

	if(m_Surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	if(m_pWindow)
		delete m_pWindow;

	if(m_Instance != VK_NULL_HANDLE)
		vkDestroyInstance(m_Instance, nullptr);
}

void VulkanApp::WindowUpdate()
{
	m_pWindow->Tick();
}


bool VulkanApp::CreateInstance(const PreDeviceSetupParameters& params)
{
	std::vector<const char*> layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> extensions = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	uint32_t count = 0;
	vkEnumerateInstanceLayerProperties(&count, nullptr);
	std::vector<VkLayerProperties> validationLayers(static_cast<size_t>(count));
	vkEnumerateInstanceLayerProperties(&count, validationLayers.data());

	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> instanceExtensions(static_cast<size_t>(count));
	vkEnumerateInstanceExtensionProperties(nullptr, &count, instanceExtensions.data());

	// Append validation layers
	for (const char* paramLayer : params.ValidationLayers)
	{
		for (const auto& layer : layers)
		{
			if (strcmp(paramLayer, layer) != 0)
			{
				layers.push_back(paramLayer);
				break;
			}
		}
	}

	// Append extensions
	for (const char* paramExtensions : params.InstanceExtensions)
	{
		for (const auto& extension : extensions)
		{
			if (strcmp(paramExtensions, extension) != 0)
			{
				layers.push_back(paramExtensions);
				break;
			}
		}
	}

	// Query if all instance layers are present
	bool allLayersPresent = true;
	const char* failedLayer = nullptr;
	for (const auto& requestedLayer : layers)
	{
		bool layerPresent = false;
		for (const auto& instanceLayer : validationLayers)
		{
			if (strcmp(instanceLayer.layerName, requestedLayer) == 0)
			{
				layerPresent = true;
				break;
			}
		}

		if (!layerPresent)
		{
			allLayersPresent = false;
			failedLayer = requestedLayer;
			break;
		}
	}

	if (!allLayersPresent)
	{
		printf("not all requested validation layers are available failed at layer request[\"%s\"]\n", failedLayer);
		return false;
	}

	// Query if all extensions are present
	bool allExtensionsPresent = true;
	const char* failedExtension = nullptr;
	for (const auto& requestedExtension : extensions)
	{
		bool extensionPresent = false;
		for (const auto& instanceExtension : instanceExtensions)
		{
			if (strcmp(instanceExtension.extensionName, requestedExtension) == 0)
			{
				extensionPresent = true;
				break;
			}
		}

		if (!extensionPresent)
		{
			allExtensionsPresent = false;
			failedExtension = requestedExtension;
			break;
		}
	}

	if (!allExtensionsPresent)
	{
		printf("not all requested instance extensions are available request[\"%s\"]\n", failedExtension);
		return false;
	}

	VkApplicationInfo info{};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pEngineName = "VkBoilerEngine";
	info.engineVersion = 1;
	info.pApplicationName = params.AppName.c_str();
	info.applicationVersion = 1;
	info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

	VkInstanceCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ci.pApplicationInfo = &info;
	ci.enabledLayerCount = 0;
	ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	ci.ppEnabledExtensionNames = extensions.data();
	if (params.EnableDeviceDebugging)
	{
		ci.enabledLayerCount = static_cast<uint32_t>(layers.size());
		ci.ppEnabledLayerNames = layers.data();
	}

	if (vkCreateInstance(&ci, nullptr, &m_Instance) != VK_SUCCESS)
	{
		printf("Failed to create instance!\n"); 
		return false;
	}
	return true;
}

bool VulkanApp::CreateWindow(const PreDeviceSetupParameters& params)
{
	m_pWindow = new Window(params.AppName, params.AllowWindowResizing, params.WindowWidth, params.WindowHeight);

	 bool success = m_pWindow != nullptr;
	if(success)
		success &= m_pWindow->IsValid();

	if (!success)
		printf("Failed to create window!\n");

	return success;
}

bool VulkanApp::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	ci.hinstance = GetModuleHandle(NULL);
	ci.hwnd = m_pWindow->GetWindowHandle();

	if (vkCreateWin32SurfaceKHR(m_Instance, &ci, nullptr, &m_Surface) != VK_SUCCESS)
	{
		printf("Failed to create window surface!\n");
		return false;
	}
	return true;
}

bool VulkanApp::PickPhysicalDevice(const PreDeviceSetupParameters& params)
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(m_Instance, &count, nullptr);
	std::vector<VkPhysicalDevice> devices(static_cast<size_t>(count));
	vkEnumeratePhysicalDevices(m_Instance, &count, devices.data());

	uint32_t deviceCount = count;

	const float LOW_WEIGHT = 1.0f;
	const float MEDIUM_WEIGHT = 2.0f;
	const float HIGH_WEIGHT = 3.0f;
	float highestScore = 0.0f;

	std::vector<std::string> GPUNames(count);
	std::vector<bool> failedOnExtensions(count);
	std::vector<bool> failedOnFeatures(count);
	std::vector<bool> failedOnQueues(count);

	uint32_t index = 0;
	for (VkPhysicalDevice device : devices)
	{
		VkPhysicalDeviceProperties props{};
		VkPhysicalDeviceFeatures features{};
		VkSurfaceCapabilitiesKHR surface_cap{};
		vkGetPhysicalDeviceProperties(device, &props);
		vkGetPhysicalDeviceFeatures(device, &features);
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &surface_cap);

		GPUNames[index] = props.deviceName;

		bool deviceSuitable = true;

		if (params.DeviceExtensions.size() > 0)
		{
			count = 0;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
			std::vector<VkExtensionProperties> extensions(count);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());

			for (size_t i = 0; i < params.DeviceExtensions.size(); i++)
			{
				bool extensionAvailable = false;
				for (const auto& extension : extensions)
				{
					if (strcmp(extension.extensionName, params.DeviceExtensions[i]) == 0)
					{
						extensionAvailable = true;
						break;
					}
				}

				if (!extensionAvailable)
				{
					deviceSuitable = false;
					failedOnExtensions[index] = true;
					break;
				}
			}
		}
		
		if (!QueryPhysicalDeviceQueues(device, params))
		{
			failedOnQueues[index] = true;
			deviceSuitable = false;
		}

		// Don't know how safe this is. Vulkan specifications doesn't state that the VkPhysicalDeviceFeatures only consists of VkBool32s
		size_t featureCount = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
		for (size_t i = 0; i < featureCount; i++)
		{
			const VkBool32& desiredValue = *(reinterpret_cast<const VkBool32*>(&params.EnabledDeviceFeatures) + i);
			const VkBool32& deviceValue = *(reinterpret_cast<VkBool32*>(&features) + i);
			if (desiredValue == VK_TRUE)
			{
				if (deviceValue != VK_TRUE)
				{
					deviceSuitable = false;
					failedOnFeatures[index] = true;
					break;
				}
			}
		}

		index++;

		if (!deviceSuitable)
			continue;
		
		float score = 1.0f;
		if (props.deviceType & VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += HIGH_WEIGHT;

		if (score > highestScore)
		{
			highestScore = score;
			m_PhysDevice = device;
		}
	}


	if (m_PhysDevice == VK_NULL_HANDLE)
	{
		printf("Failed to find suitable physical device!\n");
		for (uint32_t i = 0; i < deviceCount; i++)
		{
			printf("\t- GPU[\"%s\"] failed on:\n\t\t", GPUNames[i].c_str());
			if (failedOnFeatures[i])
				printf("[features] ");
			if (failedOnExtensions[i])
				printf("[extensions] ");
			if (failedOnQueues[i])
				printf("[queues] ");
			printf("\n");
		}

		printf("\nRecommend to look at vulkan.gpuinfo.org to find information about GPUs using device names displayed\n");
		return false;
	}

	VkPhysicalDeviceProperties props{};
	vkGetPhysicalDeviceProperties(m_PhysDevice, &props);
	printf("GPU: %s\n", props.deviceName);

	return true;
}

bool VulkanApp::QueryPhysicalDeviceQueues(VkPhysicalDevice device, const PreDeviceSetupParameters& params, 
	std::vector<QueueIndices>* pQueueIndices, std::vector<VkDeviceQueueCreateInfo>* pQueueCis)
{
	// Find queue families on device
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(static_cast<size_t>(count));
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	// Cache desired amount of queues per type
	size_t desiredQueueCount = static_cast<uint32_t>(params.DesiredQueues.size());
	std::vector<uint32_t> desiredCounts(desiredQueueCount, 0);
	for (size_t i = 0; i < params.DesiredQueues.size(); i++)
		desiredCounts[i] = params.DesiredQueues[i].Count;

	// [Initilization] Store information on which queue handles may be retrieved on which family and from which indices
	if (pQueueIndices)
	{
		pQueueIndices->resize(desiredQueueCount);

		for (size_t i = 0; i < pQueueIndices->size(); i++)
		{
			auto& index = (*pQueueIndices)[i];
			index.Types = params.DesiredQueues[i].Types;
			index.Count = {};
			index.Families = {};
			index.FirstIndex = {};
			index.FamilyCount = 0;
		}
	}

	for (uint32_t familyIndex = 0; familyIndex < static_cast<uint32_t>(families.size()); familyIndex++)
	{
		// Amount of queues in this family
		uint32_t familyQueueCount = families[familyIndex].queueCount;
		uint32_t queuesLeftInFamily = families[familyIndex].queueCount;

		// Existence of queueFamily should indicate that at least 1 queue is available
		if (queuesLeftInFamily != 0)
		{
			// Loop through desired queue types
			for (uint32_t i = 0; i < desiredQueueCount; i++)
			{
				const QueueType& queue = params.DesiredQueues[i];
				// proceed if still looking for available queues
				if (desiredCounts[i] > 0)
				{
					// Get offset in queue family
					uint32_t offset = familyQueueCount - queuesLeftInFamily;
					uint32_t count = 0;
					if (queue.Types & families[familyIndex].queueFlags)
					{
						// Amount of queues left is less than the desired queues
						if (queuesLeftInFamily < desiredCounts[i])
						{
							count = queuesLeftInFamily;

							desiredCounts[i] -= count;
							queuesLeftInFamily = 0;
						}
						// Amount of queues left is more or exactly equal to the desired queues
						else
						{
							queuesLeftInFamily -= desiredCounts[i];

							desiredCounts[i] = 0;
						}

						// [Populating] Store information on which queue handles may be retrieved on which family and from which indices
						if (pQueueIndices)
						{
							(*pQueueIndices)[i].Families.push_back(familyIndex);
							(*pQueueIndices)[i].Count.push_back(count);
							(*pQueueIndices)[i].FirstIndex.push_back(offset);
							(*pQueueIndices)[i].FamilyCount++;
						}
					}
				}
			}
		}

		if (pQueueCis)
		{
			if (queuesLeftInFamily != families[familyIndex].queueCount)
			{
				VkDeviceQueueCreateInfo queueCi{};
				queueCi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCi.queueFamilyIndex = familyIndex;
				queueCi.queueCount = familyQueueCount - queuesLeftInFamily;
				pQueueCis->push_back(queueCi);
			}
		}
	}

	bool valid = true;
	for (uint32_t i = 0; i < desiredQueueCount; i++)
	{
		if (desiredCounts[i] > 0)
			valid = false;
	}

	return valid;
}

#undef max

bool VulkanApp::CreateLogicalDevice(const PreDeviceSetupParameters& params)
{
	std::vector<VkDeviceQueueCreateInfo> queueCis;
	QueryPhysicalDeviceQueues(m_PhysDevice, params, &m_QueueIndices, &queueCis);

	uint32_t highestQueueCount = 0;
	m_TotalQueueCount = 0;
	for (const auto& queueCi : queueCis)
	{
		highestQueueCount = std::max(highestQueueCount, queueCi.queueCount);
		m_TotalQueueCount += queueCi.queueCount;
	}

	// Set all queue priorites to 1.0f
	std::vector<float> queuePriorities(highestQueueCount, 1.0f);
	for (auto& queueCi : queueCis)
		queueCi.pQueuePriorities = queuePriorities.data();

	VkDeviceCreateInfo deviceCi{};
	deviceCi.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCi.queueCreateInfoCount = static_cast<uint32_t>(queueCis.size());
	deviceCi.pQueueCreateInfos = queueCis.data();
	deviceCi.pEnabledFeatures = &params.EnabledDeviceFeatures;

	if (vkCreateDevice(m_PhysDevice, &deviceCi, nullptr, &m_Device) != VK_SUCCESS)
	{
		printf("Failed to create logical device!\n");
		return false;
	}

	return true;
}