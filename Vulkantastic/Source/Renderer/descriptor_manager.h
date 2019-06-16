#pragma once
#include "vulkan/vulkan_core.h"
#include "shader.h"
#include "buffer.h"
#include "image_view.h"
#include "sampler.h"
#include "uniform_buffer.h"

enum class PipelineType : uint8_t;
class DescriptorInst;

class DescriptorManager
{
	friend DescriptorInst;
public:
	DescriptorManager(std::vector<Shader*> Shaders);
	~DescriptorManager();

	DescriptorManager(const DescriptorManager& Rhs) = delete;
	DescriptorManager& operator=(const DescriptorManager& Rhs) = delete;

	DescriptorManager(DescriptorManager&& Rhs) noexcept;
	DescriptorManager& operator=(DescriptorManager&& Rhs) noexcept;

	std::unique_ptr<class DescriptorInst> GetDescriptorInstance(uint32_t SetIdx = 0);
	std::unique_ptr<class ShaderParameters> GetShaderParametersInstance(uint32_t SetIdx = 0);

	inline VkDescriptorSetLayout GetLayout(uint32_t SetIdx = 0) { return mLayouts[SetIdx]; }
	inline VkDescriptorPool GetPool(uint32_t SetIdx = 0) { return mPools[SetIdx]; }
	inline std::vector<Uniform> GetUniforms(uint32_t SetIdx = 0) { return mUniforms[SetIdx]; }
	inline std::vector<Uniform> GetPushConstantsForShader(ShaderType Type) { return mPushConstants[Type]; }
	inline std::map<ShaderType, std::vector<Uniform>> GetPushConstants() const { return mPushConstants;	}
	inline std::vector<Shader*> GetShaders() const { return mShaders; }
	inline uint32_t GetLayoutsCount() const { return static_cast<uint32_t>(mLayouts.size()); }

	std::vector<VkDescriptorSetLayout> GetLayouts() const;
	PipelineType GetPipelineType() const;

private:
	using DescSetLayouts = std::map<uint32_t, VkDescriptorSetLayout>;
	using DescPools = std::map<uint32_t, VkDescriptorPool>;
	using Uniforms = std::map<uint32_t, std::vector<Uniform>>;

	DescSetLayouts mLayouts;
	DescPools mPools;
	int32_t mCurrentInstanceCount = 0;
	Uniforms mUniforms;
	std::map<ShaderType, std::vector<Uniform>> mPushConstants;
	PipelineType mPipelineType;
	std::vector<Shader*> mShaders;

};

class DescriptorInst
{
	friend DescriptorManager;

public:
	~DescriptorInst();

	DescriptorInst(const DescriptorInst& Rhs) = delete;
	DescriptorInst& operator=(const DescriptorInst& Rhs) = delete;

	DescriptorInst(DescriptorInst&& Rhs) noexcept;
	DescriptorInst& operator=(DescriptorInst&& Rhs) noexcept;

	inline VkDescriptorSet GetSet() const { return mSet; }

	DescriptorInst* SetBuffer(int32_t Binding, const UniformBuffer* BufferToSet);
	DescriptorInst* SetImage(int32_t Binding, const ImageView* View, const Sampler* ImageSampler, uint32_t Index = 0);
	DescriptorInst* SetImage(int32_t Binding, const ImageView* View, uint32_t Index = 0);
	DescriptorInst* SetSampler(int32_t Binding, const Sampler* ImageSampler, uint32_t Index = 0);

	inline uint32_t GetSetIndex() const { return mSetIdx; }

	void Update();

private:
	DescriptorInst(DescriptorManager* DescManager, uint32_t SetIdx = 0);

	void AddBufferWriteDesc(const Uniform& Template);
	void AddImageWriteDesc(const Uniform& Template);

	VkDescriptorSet mSet = nullptr;

	using BufferWriteDescList = std::vector< std::pair<VkWriteDescriptorSet, VkDescriptorBufferInfo> >;
	using ImageWriteDescList = std::vector< std::pair<VkWriteDescriptorSet, std::vector<VkDescriptorImageInfo>> >;

	BufferWriteDescList mBuffersInfo;
	ImageWriteDescList mImagesInfo;

	std::vector<Uniform> mUniforms;

	DescriptorManager* mOwner = nullptr;
	uint32_t mSetIdx = 0;

};

using upDescriptorInst = std::unique_ptr<DescriptorInst>;
using upDescriptorInstList = std::vector<upDescriptorInst>;