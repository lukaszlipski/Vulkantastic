#include "descriptor_manager.h"
#include "device.h"
#include "../Utilities/assert.h"
#include "core.h"

constexpr int32_t MaxInstances = 128;

DescriptorManager::DescriptorManager(std::initializer_list<Shader*> Shaders)
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	std::vector<VkDescriptorSetLayoutBinding> DescLayoutBindings;
	std::vector<VkDescriptorPoolSize> DescPoolSizes;

	for (auto& Shader : Shaders)
	{
		auto Uniforms = Shader->GetUniforms();

		for (auto& Uniform : Uniforms)
		{
			VkDescriptorSetLayoutBinding Binding = {};
			Binding.binding = Uniform.Binding;
			Binding.descriptorCount = 1;
			Binding.stageFlags = ShaderReflection::InternalShaderTypeToVulkan(Shader->GetType());
			Binding.descriptorType = ShaderReflection::InternalUniformTypeToVulkan(Uniform.Format);

			DescLayoutBindings.push_back(Binding);

			VkDescriptorPoolSize Size = {};
			Size.type = Binding.descriptorType;
			Size.descriptorCount = 1;

			DescPoolSizes.push_back(Size);

		}

	}

	VkDescriptorSetLayoutCreateInfo DescriptorLayoutInfo = {};
	DescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorLayoutInfo.bindingCount = static_cast<uint32_t>(DescLayoutBindings.size());
	DescriptorLayoutInfo.pBindings = DescLayoutBindings.data();
	
	Assert(vkCreateDescriptorSetLayout(Device, &DescriptorLayoutInfo, nullptr, &mLayout) == VK_SUCCESS);

	VkDescriptorPoolCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	CreateInfo.poolSizeCount = static_cast<uint32_t>(DescPoolSizes.size());
	CreateInfo.pPoolSizes = DescPoolSizes.data();
	CreateInfo.maxSets = MaxInstances;

	vkCreateDescriptorPool(Device, &CreateInfo, nullptr, &mPool);
}

DescriptorManager::DescriptorManager(DescriptorManager&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

std::unique_ptr<DescriptorInst> DescriptorManager::GetDescriptorInstance()
{
	Assert(CurrentInstanceCount++ <= MaxInstances);

	return std::unique_ptr<DescriptorInst>(new DescriptorInst(this));
}

DescriptorManager& DescriptorManager::operator=(DescriptorManager&& Rhs) noexcept
{
	mLayout = Rhs.mLayout;
	Rhs.mLayout = nullptr;

	mPool = Rhs.mPool;
	Rhs.mPool = nullptr;

	return *this;
}

DescriptorManager::~DescriptorManager()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkDestroyDescriptorSetLayout(Device, mLayout, nullptr);
}

DescriptorInst::DescriptorInst(DescriptorManager* DescManager)
{
	Assert(DescManager);
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkDescriptorSetAllocateInfo AllocDescriptorSetInfo = {};
	AllocDescriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocDescriptorSetInfo.descriptorPool = DescManager->GetPool();
	AllocDescriptorSetInfo.descriptorSetCount = 1;

	auto DescLayout = DescManager->GetLayout();
	AllocDescriptorSetInfo.pSetLayouts = &DescLayout;

	vkAllocateDescriptorSets(Device, &AllocDescriptorSetInfo, &mSet);
}
