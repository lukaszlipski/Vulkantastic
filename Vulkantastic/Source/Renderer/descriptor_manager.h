#pragma once
#include "vulkan/vulkan_core.h"
#include "shader.h"

class DescriptorManager
{
public:
	DescriptorManager(std::initializer_list<Shader*> Shaders);
	~DescriptorManager();

	DescriptorManager(const DescriptorManager& Rhs) = delete;
	DescriptorManager& operator=(const DescriptorManager& Rhs) = delete;

	DescriptorManager(DescriptorManager&& Rhs) noexcept;
	DescriptorManager& operator=(DescriptorManager&& Rhs) noexcept;

	std::unique_ptr<class DescriptorInst> GetDescriptorInstance();

	inline VkDescriptorSetLayout GetLayout() const { return mLayout; }
	inline VkDescriptorPool GetPool() const { return mPool; }

private:
	VkDescriptorSetLayout mLayout = nullptr;
	VkDescriptorPool mPool = nullptr;
	int32_t CurrentInstanceCount = 0;

};

class DescriptorInst
{
	friend DescriptorManager;
public:
	inline VkDescriptorSet GetSet() const { return mSet; }

private:
	DescriptorInst(DescriptorManager* DescManager);

	VkDescriptorSet mSet = nullptr;
};
