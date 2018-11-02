#pragma once
#include "vulkan/vulkan_core.h"
#include "shader.h"
#include "buffer.h"
#include "image_view.h"
#include "sampler.h"
#include <map>

enum class PipelineType : uint8_t;

class DescriptorManager
{
public:
	DescriptorManager(std::vector<Shader*> Shaders);
	~DescriptorManager();

	DescriptorManager(const DescriptorManager& Rhs) = delete;
	DescriptorManager& operator=(const DescriptorManager& Rhs) = delete;

	DescriptorManager(DescriptorManager&& Rhs) noexcept;
	DescriptorManager& operator=(DescriptorManager&& Rhs) noexcept;

	std::unique_ptr<class DescriptorInst> GetDescriptorInstance();

	inline VkDescriptorSetLayout GetLayout() const { return mLayout; }
	inline VkDescriptorPool GetPool() const { return mPool; }
	inline std::vector<Uniform> GetUniforms() const { return mUniforms; }
	inline std::vector<Uniform> GetPushConstantsForShader(ShaderType Type) { return mPushConstants[Type]; }
	inline std::map<ShaderType, std::vector<Uniform>> GetPushConstants() const { return mPushConstants;	}

	PipelineType GetPipelineType() const;

private:
	VkDescriptorSetLayout mLayout = nullptr;
	VkDescriptorPool mPool = nullptr;
	int32_t mCurrentInstanceCount = 0;
	std::vector<Uniform> mUniforms;
	std::map<ShaderType, std::vector<Uniform>> mPushConstants;
	PipelineType mPipelineType;

};

class DescriptorInst
{
	friend DescriptorManager;

	using UniformBuffersList = std::vector<std::unique_ptr<class UniformBuffer>>;
	using PushConstantBuffersList = std::map<ShaderType, std::unique_ptr<class PushConstantBuffer>>;

public:
	~DescriptorInst();

	DescriptorInst(const DescriptorInst& Rhs) = delete;
	DescriptorInst& operator=(const DescriptorInst& Rhs) = delete;

	DescriptorInst(DescriptorInst&& Rhs) noexcept;
	DescriptorInst& operator=(DescriptorInst&& Rhs) noexcept;

	inline VkDescriptorSet GetSet() const { return mSet; }

	DescriptorInst* SetBuffer(const std::string& Name, const Buffer& BufferToSet);
	DescriptorInst* SetImage(const std::string& Name, const ImageView& View, const Sampler& ImageSampler);

	class UniformBuffer* GetUniformBuffer(const std::string& Name);
	class PushConstantBuffer* GetPushConstantBuffer(ShaderType Type);

	void Update();

private:
	DescriptorInst(DescriptorManager* DescManager);

	VkDescriptorSet mSet = nullptr;

	std::vector<VkWriteDescriptorSet> mWriteSets;
	std::vector<VkDescriptorBufferInfo> mBuffersInfo;
	std::vector<VkDescriptorImageInfo> mImagesInfo;

	std::vector<Uniform> mUniforms;
	std::map<ShaderType, std::vector<Uniform>> mPushConstants;

	UniformBuffersList mUniformBuffers;
	PushConstantBuffersList mPushConstantBuffers;
};
