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

	
	using BindingsList = std::vector<VkDescriptorSetLayoutBinding>;
	using PoolSizeList = std::vector<VkDescriptorPoolSize>;

	std::map<uint32_t, BindingsList> DescLayoutBindings;
	std::map<uint32_t, PoolSizeList> DescPoolSizes;

	for (auto& Shader : mShaders)
	{
		auto Uniforms = Shader->GetUniforms();
		auto PushConstants = Shader->GetPushConstants();

		mPushConstants[Shader->GetType()] = Shader->GetPushConstants();

		for (auto& Uniform : Uniforms)
		{
			mUniforms[Uniform.Set].push_back(Uniform);

			VkDescriptorSetLayoutBinding Binding = {};
			Binding.binding = Uniform.Binding;
			Binding.descriptorCount = Uniform.Size;
			Binding.stageFlags = ShaderReflection::InternalShaderTypeToVulkan(Shader->GetType());
			Binding.descriptorType = ShaderReflection::InternalUniformTypeToVulkan(Uniform.Format);

			DescLayoutBindings[Uniform.Set].push_back(Binding);

			VkDescriptorPoolSize Size = {};
			Size.type = Binding.descriptorType;
			Size.descriptorCount = Uniform.Size;

			DescPoolSizes[Uniform.Set].push_back(Size);

		}
	}

	for (const auto& Binding : DescLayoutBindings)
	{
		uint32_t SetIdx = Binding.first;

		const BindingsList& Bindings = DescLayoutBindings[SetIdx];
		const PoolSizeList& PoolSizes = DescPoolSizes[SetIdx];

		VkDescriptorSetLayoutCreateInfo DescriptorLayoutInfo = {};
		DescriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		DescriptorLayoutInfo.bindingCount = static_cast<uint32_t>(Bindings.size());
		DescriptorLayoutInfo.pBindings = Bindings.data();
	
		Assert(vkCreateDescriptorSetLayout(Device, &DescriptorLayoutInfo, nullptr, &mLayouts[SetIdx]) == VK_SUCCESS);

		VkDescriptorPoolCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		CreateInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
		CreateInfo.pPoolSizes = PoolSizes.data();
		CreateInfo.maxSets = MaxInstances;
		CreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		Assert(vkCreateDescriptorPool(Device, &CreateInfo, nullptr, &mPools[SetIdx]) == VK_SUCCESS);
	}

}

DescriptorManager::DescriptorManager(DescriptorManager&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

std::unique_ptr<DescriptorInst> DescriptorManager::GetDescriptorInstance(uint32_t SetIdx)
{
	Assert(mCurrentInstanceCount++ <= MaxInstances);

	return std::unique_ptr<DescriptorInst>(new DescriptorInst(this, SetIdx));
}

std::unique_ptr<ShaderParameters> DescriptorManager::GetShaderParametersInstance(uint32_t SetIdx)
{
	PipelineManager::KeyType Key = PipelineManager::Get().HashShaders(GetShaders());
	return std::make_unique<ShaderParameters>(Key, GetUniforms(SetIdx), GetPushConstants() );
}

std::vector<VkDescriptorSetLayout> DescriptorManager::GetLayouts() const
{
	std::vector<VkDescriptorSetLayout> Result;
	Result.reserve(GetLayoutsCount());
	for (const auto& Layout : mLayouts)
	{
		Result.push_back(Layout.second);
	}
	return Result;
}

PipelineType DescriptorManager::GetPipelineType() const
{
	return mPipelineType;
}

DescriptorManager& DescriptorManager::operator=(DescriptorManager&& Rhs) noexcept
{
	mLayouts = Rhs.mLayouts;
	Rhs.mLayouts.clear();

	mPools = Rhs.mPools;
	Rhs.mPools.clear();

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

	for (auto& Layout : mLayouts)
	{
		uint32_t SetIdx = Layout.first;
		vkDestroyDescriptorSetLayout(Device, mLayouts[SetIdx], nullptr);
		vkDestroyDescriptorPool(Device, mPools[SetIdx], nullptr);
	}

}

DescriptorInst::~DescriptorInst()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();
	vkFreeDescriptorSets(Device, mOwner->GetPool(mSetIdx), 1, &mSet);

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

DescriptorInst::DescriptorInst(DescriptorManager* DescManager, uint32_t SetIdx)
	: mOwner(DescManager), mSetIdx(SetIdx)
{
	Assert(mOwner);
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkDescriptorSetAllocateInfo AllocDescriptorSetInfo = {};
	AllocDescriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocDescriptorSetInfo.descriptorPool = mOwner->GetPool(mSetIdx);
	AllocDescriptorSetInfo.descriptorSetCount = 1;

	auto DescLayout = mOwner->GetLayout(mSetIdx);
	AllocDescriptorSetInfo.pSetLayouts = &DescLayout;

	Assert(vkAllocateDescriptorSets(Device, &AllocDescriptorSetInfo, &mSet) == VK_SUCCESS);
	
	mUniforms = mOwner->GetUniforms(mSetIdx);

	auto BuffersCount = std::count_if(mUniforms.begin(), mUniforms.end(), [](const auto& Elem) {
		return Elem.Format == VariableType::STRUCTURE || Elem.Format == VariableType::BUFFER;
	});

	auto ImagesCount = std::count_if(mUniforms.begin(), mUniforms.end(), [](const auto& Elem) {
		return Elem.Format == VariableType::COMBINED;
	});

	mBuffersInfo.reserve(BuffersCount);
	mImagesInfo.reserve(ImagesCount);

	for (const Uniform& Template : mUniforms)
	{
		if (Template.Format == VariableType::STRUCTURE || Template.Format == VariableType::BUFFER)
		{
			AddBufferWriteDesc(Template);
		}
		else if (Template.Format == VariableType::COMBINED)
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
