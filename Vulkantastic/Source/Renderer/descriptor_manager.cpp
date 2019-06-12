#include "descriptor_manager.h"
#include "device.h"
#include "../Utilities/assert.h"
#include "core.h"
#include <algorithm>
#include "uniform_buffer.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "uniform_raw_data.h"
#include "pipeline_manager.h"
#include "shader_parameters.h"

constexpr int32_t MaxInstances = 128;

DescriptorManager::DescriptorManager(std::vector<Shader*> Shaders)
	: mShaders(Shaders)
{
	Assert(Shaders.size());

	if (Shaders[0]->GetType() == ShaderType::COMPUTE)
	{
		mPipelineType = PipelineType::COMPUTE;
	}
	else
	{
		mPipelineType = PipelineType::GRAPHICS;
	}

	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	std::vector<VkDescriptorSetLayoutBinding> DescLayoutBindings;
	std::vector<VkDescriptorPoolSize> DescPoolSizes;

	for (auto& Shader : mShaders)
	{
		auto Uniforms = Shader->GetUniforms();
		auto PushConstants = Shader->GetPushConstants();

		mUniforms.insert(mUniforms.end(), Uniforms.begin(), Uniforms.end());
		mPushConstants[Shader->GetType()] = Shader->GetPushConstants();

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
	CreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	Assert(vkCreateDescriptorPool(Device, &CreateInfo, nullptr, &mPool) == VK_SUCCESS);

}

DescriptorManager::DescriptorManager(DescriptorManager&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

std::unique_ptr<DescriptorInst> DescriptorManager::GetDescriptorInstance()
{
	Assert(mCurrentInstanceCount++ <= MaxInstances);

	return std::unique_ptr<DescriptorInst>(new DescriptorInst(this));
}

std::unique_ptr<ShaderParameters> DescriptorManager::GetShaderParametersInstance()
{
	PipelineManager::KeyType Key = PipelineManager::Get().HashShaders(GetShaders());
	return std::make_unique<ShaderParameters>(Key, GetUniforms(), GetPushConstants() );
}

PipelineType DescriptorManager::GetPipelineType() const
{
	return mPipelineType;
}

DescriptorManager& DescriptorManager::operator=(DescriptorManager&& Rhs) noexcept
{
	mLayout = Rhs.mLayout;
	Rhs.mLayout = nullptr;

	mPool = Rhs.mPool;
	Rhs.mPool = nullptr;

	mUniforms = std::move(Rhs.mUniforms);
	mPushConstants = std::move(Rhs.mPushConstants);
	mShaders = std::move(Rhs.mShaders);

	mCurrentInstanceCount = Rhs.mCurrentInstanceCount;
	mPipelineType = Rhs.mPipelineType;

	return *this;
}

DescriptorManager::~DescriptorManager()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	vkDestroyDescriptorSetLayout(Device, mLayout, nullptr);
	vkDestroyDescriptorPool(Device, mPool, nullptr);

}

DescriptorInst::~DescriptorInst()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	vkFreeDescriptorSets(Device, mOwner->GetPool(), 1, &mSet);

	mOwner->mCurrentInstanceCount--;
}

DescriptorInst* DescriptorInst::SetBuffer(int32_t Binding, const UniformBuffer* BufferToSet)
{
	
	auto SetIt = std::find_if(mBuffersInfo.begin(), mBuffersInfo.end(), [&Binding](const auto& Elem) {
		return Elem.first.dstBinding == Binding;
	});

	if (SetIt != mBuffersInfo.end() && BufferToSet)
	{
		VkDescriptorBufferInfo& BufferInfo = SetIt->second;
		BufferInfo.buffer = BufferToSet->GetBuffer()->GetBuffer();
		BufferInfo.range = BufferToSet->GetAlignmentSize();
	}

	return this;
}

DescriptorInst* DescriptorInst::SetImage(int32_t Binding, const ImageView* View, const Sampler* ImageSampler)
{
	auto SetIt = std::find_if(mImagesInfo.begin(), mImagesInfo.end(), [&Binding](const auto& Elem) {
		return Elem.first.dstBinding == Binding;
		});

	if (SetIt != mImagesInfo.end() && View && ImageSampler)
	{
		VkDescriptorImageInfo& ImageInfo = SetIt->second;
		ImageInfo.imageLayout = static_cast<VkImageLayout>(View->GetCurrentImageLayout());
		ImageInfo.imageView = View->GetView();
		ImageInfo.sampler = ImageSampler->GetSampler();
	}

	return this;
}


void DescriptorInst::Update()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	std::vector<VkWriteDescriptorSet> Sets;
	Sets.reserve(mBuffersInfo.size() + mImagesInfo.size());

	for (const auto& BufferInfo : mBuffersInfo)
	{
		Sets.push_back(BufferInfo.first);
	}

	for (const auto& ImageInfo : mImagesInfo)
	{
		Sets.push_back(ImageInfo.first);
	}

	vkUpdateDescriptorSets(Device, static_cast<uint32_t>(Sets.size()), Sets.data(), 0, nullptr);

}

DescriptorInst::DescriptorInst(DescriptorManager* DescManager)
	: mOwner(DescManager)
{
	Assert(mOwner);
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkDescriptorSetAllocateInfo AllocDescriptorSetInfo = {};
	AllocDescriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocDescriptorSetInfo.descriptorPool = mOwner->GetPool();
	AllocDescriptorSetInfo.descriptorSetCount = 1;

	auto DescLayout = mOwner->GetLayout();
	AllocDescriptorSetInfo.pSetLayouts = &DescLayout;

	Assert(vkAllocateDescriptorSets(Device, &AllocDescriptorSetInfo, &mSet) == VK_SUCCESS);
	
	mUniforms = mOwner->GetUniforms();

	auto BuffersCount = std::count_if(mUniforms.begin(), mUniforms.end(), [](const auto& Elem) {
		return Elem.Format == VariableType::STRUCTURE || Elem.Format == VariableType::BUFFER;
	});

	auto ImagesCount = std::count_if(mUniforms.begin(), mUniforms.end(), [](const auto& Elem) {
		return Elem.Format == VariableType::SAMPLER;
	});

	mBuffersInfo.reserve(BuffersCount);
	mImagesInfo.reserve(ImagesCount);

	for (const Uniform& Template : mUniforms)
	{
		if (Template.Format == VariableType::STRUCTURE || Template.Format == VariableType::BUFFER)
		{
			AddBufferWriteDesc(Template);
		}
		else if (Template.Format == VariableType::SAMPLER)
		{
			AddImageWriteDesc(Template);
		}
	}

}

DescriptorInst::DescriptorInst(DescriptorInst&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

void DescriptorInst::AddBufferWriteDesc(const Uniform& Template)
{
	mBuffersInfo.push_back({});

	auto& Entry = mBuffersInfo.back();
	auto& Set = Entry.first;
	
	Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	Set.dstSet = mSet;
	Set.dstBinding = Template.Binding;
	Set.descriptorType = Template.Format == VariableType::STRUCTURE ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	Set.descriptorCount = 1;
	Set.pBufferInfo = &Entry.second;
}

void DescriptorInst::AddImageWriteDesc(const Uniform& Template)
{
	mImagesInfo.push_back({});

	auto& Entry = mImagesInfo.back();
	auto& Set = Entry.first;

	Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	Set.dstSet = mSet;
	Set.dstBinding = Template.Binding;
	Set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	Set.descriptorCount = 1;
	Set.pImageInfo = &Entry.second;
}

DescriptorInst& DescriptorInst::operator=(DescriptorInst&& Rhs) noexcept
{
	mSet = Rhs.mSet;
	Rhs.mSet = nullptr;

	mBuffersInfo = std::move(Rhs.mBuffersInfo);
	mImagesInfo = std::move(Rhs.mImagesInfo);

	return *this;
}
