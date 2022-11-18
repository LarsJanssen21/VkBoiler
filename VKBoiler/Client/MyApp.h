#pragma once

#include "Template/App.h"

class MyApp final : public VulkanApp
{
public:
	MyApp() { }
	~MyApp() { }

	virtual void PreDeviceSetup(PreDeviceSetupParameters& params) final override;
	virtual void Init() final override;
	virtual void Tick() final override;
	virtual void Destroy() final override;

private:
	VkQueue* m_Queues = nullptr;
};