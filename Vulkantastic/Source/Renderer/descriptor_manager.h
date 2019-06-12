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

	std::unique_ptr<class DescriptorInst> GetDescriptorInstance();
	std::unique_ptr<class ShaderParameters> GetShaderParametersInstance();

	inline VkDescriptorSetLayout GetLayout() const { return mLayout; }
	inline VkDescriptorPool GetPool() const { return mPool; }
	inline std::vector<Uniform> GetUniforms() const { return mUniforms; }
	inline std::vector<Uniform> GetPushConstantsForShader(ShaderType Type) { return mPushConstants[Type]; }
	inline std::map<ShaderType, std::vector<Uniform>> GetPushConstants() const { return mPushConstants;	}
	inline std::vector<Shader*> GetShaders() const { return mShaders; }

	PipelineType GetPipelineType() const;

private:
	VkDescriptorSetLayout mLayout = nullptr;
	VkDescriptorPool mPool = nullptr;
	int32_t mCurrentInstanceCount = 0;
	std::vector<Uniform> mUniforms;
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
	DescriptorInst* SetImage(int32_t Binding, const ImageView* View, const Sampler* ImageSampler);

	void Update();

private:
	DescriptorInst(DescriptorManager* DescManager);

	void AddBufferWriteDesc(const Uniform& Template);
	void AddImageWriteDesc(const Uniform& Template);

	VkDescriptorSet mSet = nullptr;

	using BufferWriteDescList = std::vector< std::pair<VkWriteDescriptorSet, VkDescriptorBufferInfo> >;
	using ImageWriteDescList = std::vector< std::pair<VkWriteDescriptorSet, VkDescriptorImageInfo> >;

	BufferWriteDescList mBuffersInfo;
	ImageWriteDescList mImagesInfo;

	std::vector<Uniform> mUniforms;

	DescriptorManager* mOwner = nullptr;

};

using upDescriptorInst = std::unique_ptr<DescriptorInst>;
using upDescriptorInstList = std::vector<upDescriptorInst>;