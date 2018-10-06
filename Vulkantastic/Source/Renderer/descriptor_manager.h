#pragma once
#include "vulkan/vulkan_core.h"
#include "shader.h"
#include "buffer.h"
#include "image_view.h"
#include "sampler.h"
#include <map>

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
	inline std::vector<Uniform> GetUniforms() const { return mUniforms; }
	inline std::vector<Uniform> GetPushConstantsForShader(ShaderType Type) { return mPushConstants[Type]; }
	inline std::map<ShaderType, std::vector<Uniform>> GetPushConstants() const { return mPushConstants;	}

private:
	VkDescriptorSetLayout mLayout = nullptr;
	VkDescriptorPool mPool = nullptr;
	int32_t mCurrentInstanceCount = 0;
	std::vector<Uniform> mUniforms;
	std::map<ShaderType, std::vector<Uniform>> mPushConstants;

};

class DescriptorInst
{
	friend DescriptorManager;
public:
	inline VkDescriptorSet GetSet() const { return mSet; }

	DescriptorInst* SetBuffer(const std::string& Name, const Buffer& BufferToSet);
	DescriptorInst* SetImage(const std::string& Name, const ImageView& View, const Sampler& ImageSampler);
	void Update();

private:
	DescriptorInst(DescriptorManager* DescManager);

	VkDescriptorSet mSet = nullptr;

	std::vector<VkWriteDescriptorSet> mWriteSets;
	std::vector<VkDescriptorBufferInfo> mBuffersInfo;
	std::vector<VkDescriptorImageInfo> mImagesInfo;

	std::vector<Uniform> mUniforms;
	std::map<ShaderType, std::vector<Uniform>> mPushConstants;
};
