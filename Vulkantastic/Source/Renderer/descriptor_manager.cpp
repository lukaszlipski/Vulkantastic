#include "descriptor_manager.h"
#include "device.h"
#include "../Utilities/assert.h"
#include "core.h"
#include <algorithm>
#include "uniform_buffer.h"
#include "pipeline.h"
#include "swap_chain.h"
#include "push_constant_buffer.h"
#include "pipeline_manager.h"

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
	
}

DescriptorInst* DescriptorInst::SetBuffer(const std::string& Name, const Buffer& BufferToSet)
{

	auto UniformIt = std::find_if(mUniforms.begin(), mUniforms.end(), [&Name](const auto& Elem) {
		return Elem.Name == Name;
	});

	if (UniformIt != mUniforms.end())
	{
		VkDescriptorBufferInfo BufferInfo = {};
		BufferInfo.buffer = BufferToSet.GetBuffer();
		BufferInfo.range = BufferToSet.GetSize();

		mBuffersInfo.push_back(BufferInfo);

		VkWriteDescriptorSet Set = {};
		Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Set.dstSet = mSet;
		Set.dstBinding = UniformIt->Binding;
		Set.descriptorType = UniformIt->Format == VariableType::STRUCTURE ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		Set.descriptorCount = 1;
		Set.pBufferInfo = &mBuffersInfo.back();

		auto Entry = std::find_if(mWriteSets.begin(), mWriteSets.end(), [Binding = UniformIt->Binding](const auto& Elem) {
			return Elem.dstBinding == Binding;
		});

		if (Entry != mWriteSets.end())
		{
			*Entry = Set;
		}
		else
		{
			mWriteSets.push_back(Set);
		}

	}

	return this;
}

DescriptorInst* DescriptorInst::SetImage(const std::string& Name, const ImageView& View, const Sampler& ImageSampler)
{
	auto UniformIt = std::find_if(mUniforms.begin(), mUniforms.end(), [&Name](const auto& Elem) {
		return Elem.Name == Name;
	});

	if (UniformIt != mUniforms.end())
	{
		
		VkDescriptorImageInfo ImageInfo = {};
		ImageInfo.imageLayout = static_cast<VkImageLayout>(View.GetCurrentImageLayout());
		ImageInfo.imageView = View.GetView();
		ImageInfo.sampler = ImageSampler.GetSampler();

		mImagesInfo.push_back(ImageInfo);

		VkWriteDescriptorSet Set = {};
		Set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		Set.dstSet = mSet;
		Set.dstBinding = UniformIt->Binding;
		Set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		Set.descriptorCount = 1;
		Set.pImageInfo = &mImagesInfo.back();

		auto Entry = std::find_if(mWriteSets.begin(), mWriteSets.end(), [Binding = UniformIt->Binding](const auto& Elem) {
			return Elem.dstBinding == Binding;
		});

		if (Entry != mWriteSets.end())
		{
			*Entry = Set;
		}
		else
		{
			mWriteSets.push_back(Set);
		}

		mWriteSets.push_back(Set);
	}

	return this;
}

class UniformBuffer* DescriptorInst::GetUniformBuffer(const std::string& Name)
{
	auto CurrentImageIndex = VulkanCore::Get().GetImageIndex();

	for (auto& Buffer : mUniformBuffers)
	{
		if (Buffer->GetName() == Name)
		{
			return Buffer.get();
		}
	}
	return nullptr;
}

class PushConstantBuffer* DescriptorInst::GetPushConstantBuffer(ShaderType Type)
{
	auto PushConstant = mPushConstantBuffers.find(Type);
	if (PushConstant != mPushConstantBuffers.end())
	{
		return PushConstant->second.get();
	}
	return nullptr;
}

void DescriptorInst::Update()
{
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	for (auto& Buffer : mUniformBuffers)
	{
		Buffer->Update();

		auto BufferPtr = Buffer->GetBuffer();
		SetBuffer(Buffer->GetName(), *BufferPtr);

	}

	vkUpdateDescriptorSets(Device, static_cast<uint32_t>(mWriteSets.size()), mWriteSets.data(), 0, nullptr);

	mBuffersInfo.clear();
	mImagesInfo.clear();
	mWriteSets.clear();
}

DescriptorInst::DescriptorInst(DescriptorManager* DescManager)
	: mUniforms(DescManager->GetUniforms()), mPushConstants(DescManager->GetPushConstants())
{
	Assert(DescManager);
	auto Device = VulkanCore::Get().GetDevice()->GetDevice();

	VkDescriptorSetAllocateInfo AllocDescriptorSetInfo = {};
	AllocDescriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	AllocDescriptorSetInfo.descriptorPool = DescManager->GetPool();
	AllocDescriptorSetInfo.descriptorSetCount = 1;

	auto DescLayout = DescManager->GetLayout();
	AllocDescriptorSetInfo.pSetLayouts = &DescLayout;

	Assert(vkAllocateDescriptorSets(Device, &AllocDescriptorSetInfo, &mSet) == VK_SUCCESS);

	auto BuffersCount = std::count_if(mUniforms.begin(), mUniforms.end(), [](const auto& Elem) {
		return Elem.Format == VariableType::STRUCTURE || Elem.Format == VariableType::BUFFER;
	});

	auto ImagesCount = std::count_if(mUniforms.begin(), mUniforms.end(), [](const auto& Elem) {
		return Elem.Format == VariableType::SAMPLER;
	});

	mBuffersInfo.reserve(BuffersCount);
	mImagesInfo.reserve(ImagesCount);

	uint32_t QueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;

	if (DescManager->GetPipelineType() == PipelineType::GRAPHICS)
	{
		QueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().GraphicsIndex;
	}
	else
	{
		QueueIndex = VulkanCore::Get().GetDevice()->GetQueuesIndicies().ComputeIndex;
	}

	std::for_each(mUniforms.begin(), mUniforms.end(), [&UBuffers = mUniformBuffers, QueueIndex](const auto& Elem) {
		if (Elem.Format == VariableType::STRUCTURE)
		{
			std::vector<uint32_t> Indicies = { QueueIndex };

			UBuffers.push_back(std::make_unique<UniformBuffer>(Elem, Indicies));	
		}
	});

	std::for_each(mPushConstants.begin(), mPushConstants.end(), [&PCBuffers = mPushConstantBuffers](const auto& Elem) {
		if (Elem.second.size() <= 0) { return; }
		PCBuffers[Elem.first] = std::make_unique<PushConstantBuffer>(Elem.second.front());
	});

	mPipelineKey = PipelineManager::Get().HashShaders(DescManager->GetShaders());

}

DescriptorInst::DescriptorInst(DescriptorInst&& Rhs) noexcept
{
	*this = std::move(Rhs);
}

DescriptorInst& DescriptorInst::operator=(DescriptorInst&& Rhs) noexcept
{
	mSet = Rhs.mSet;
	Rhs.mSet = nullptr;

	mWriteSets = std::move(Rhs.mWriteSets);
	mBuffersInfo = std::move(Rhs.mBuffersInfo);
	mImagesInfo = std::move(Rhs.mImagesInfo);

	mUniforms = std::move(Rhs.mUniforms);
	mPushConstants = std::move(Rhs.mPushConstants);

	mUniformBuffers = std::move(Rhs.mUniformBuffers);

	return *this;
}
