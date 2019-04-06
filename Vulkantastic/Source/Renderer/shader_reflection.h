#pragma once
#include "vulkan/vulkan_core.h"

enum class ShaderType : uint8_t
{
	VERTEX = 0,
	FRAGMENT,
	COMPUTE,
	MAX
};

enum class VariableType
{
	FLOAT = 0,
	FLOAT2,
	FLOAT3,
	FLOAT4,
	INT,
	INT2,
	INT3,
	INT4,
	MAT2x2,
	MAT3x3,
	MAT4x4,
	STRUCTURE,
	BUFFER,
	SAMPLER,
	MAX
};

struct Variable
{
	uint32_t Id = 0;
	VariableType Type = VariableType::MAX;
	uint32_t Size = 1;
	std::vector<Variable> Members;
};

struct Input
{
	VariableType Format = VariableType::MAX;
	uint32_t Location = 0;
	uint32_t Set = 0;
	std::string Name;
};

struct Output
{
	VariableType Format = VariableType::MAX;
	uint32_t Location = 0;
	std::string Name;
};

struct UniformMember
{
	VariableType Format = VariableType::MAX;
	uint32_t Offset = 0;
	uint32_t Size = 1;
	std::string Name;
	std::vector<UniformMember> Members;
};

struct Uniform
{
	VariableType Format = VariableType::MAX;
	uint32_t Binding = 0;
	uint32_t Set = 0;
	uint32_t Size = 1;
	std::string Name;
	std::vector<UniformMember> Members;
};

class ShaderReflection
{
public:
	ShaderReflection(const std::vector<uint32_t>& Source);

	ShaderReflection(std::vector<uint32_t>&& Source);

	std::vector<Input> GetInputs() const { return mInputs; }
	std::vector<Output> GetOutputs() const { return mOutputs; }
	std::vector<Uniform> GetUniforms() const { return mUniforms; }
	std::vector<Uniform> GetPushConstants() const { return mPushConstants; }
	inline ShaderType GetShaderType() const { return mType; }

	static VkFormat InternalFormatToVulkan(VariableType Format);
	static VkShaderStageFlags InternalShaderTypeToVulkan(ShaderType Type);
	static VkDescriptorType InternalUniformTypeToVulkan(VariableType Type);
	static int32_t GetSizeForFormat(VariableType Format);
	static int32_t GetSizeForStructure(Uniform Structure);

private:

	bool ParseHeader();
	bool ParseInstruction();
	void GetEntryPoint(uint32_t InstructionIndex);
	void GetName(uint32_t InstructionIndex);
	void GetMemberName(uint32_t InstructionIndex);
	void GetDecorate(uint32_t InstructionIndex);
	void GetMemberDecorate(uint32_t InstructionIndex);
	void GetType(uint32_t InstructionIndex);
	void GetConstant(uint32_t InstructionIndex);

	Input GetInput(const Variable& InputVariable);
	Output GetOutput(const Variable& OutputVariable);
	Uniform GetUniform(const Variable& UniformVariable);
	Variable GetVariable(uint32_t InstructionIndex);

	UniformMember ProcessUniformMember(const Variable& MemberVariable, uint32_t ParentId, uint32_t Index);

	VariableType GetVariableType(uint32_t Id);

	VariableType GetTypeOfStructure(uint32_t Id);

	VariableType GetTypeOfVector(uint32_t Id);
	VariableType GetTypeOfMatrix(uint32_t Id);

	uint32_t mCurrentInstruction = 0;
	std::vector<uint32_t> mSource;
	uint32_t mBound = 0;
	std::map<uint32_t, std::string> mNames;
	std::map<uint32_t, std::map<uint32_t, std::string>> mMemberNames;
	std::map<uint32_t, uint32_t> mVariablesSection;
	std::map<uint32_t, std::list<uint32_t>> mDecorateSection;
	std::map<uint32_t, std::list<uint32_t>> mMemberDecorateSection;

	std::vector<Input> mInputs;
	std::vector<Output> mOutputs;
	std::vector<Uniform> mUniforms;
	std::vector<Uniform> mPushConstants;

	std::string mEntryPoint;
	ShaderType mType = ShaderType::MAX;

};
