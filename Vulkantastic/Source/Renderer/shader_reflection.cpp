#include "shader_reflection.h"
#include <type_traits>
#include "spirv/1.1/spirv.h"
#include <utility>
#include <assert.h>

struct InstructionInfo
{
	uint32_t Bytes;
	uint32_t Instruction;

	InstructionInfo(uint32_t InstructionIndex, const std::vector<uint32_t>& Source)
	{
		Bytes = (Source[InstructionIndex] & 0xFFFF0000) >> 16;
		Instruction = Source[InstructionIndex] & 0x0000FFFF;
	}
};

ShaderReflection::ShaderReflection(std::vector<uint32_t>&& Source)
	: mSource(std::forward<std::vector<uint32_t>>(Source))
{
	
	if (!ParseHeader()) { return; }
	while (ParseInstruction()) {}

}

ShaderReflection::ShaderReflection(const std::vector<uint32_t>& Source)
	: mSource(Source)
{

	if (!ParseHeader()) { return; }
	while (ParseInstruction()) {}

}

bool ShaderReflection::ParseHeader()
{
	// Check for magic number
	if (mSource[mCurrentInstruction++] != SpvMagicNumber)
	{
		// #TODO: add to log
		return false;
	}

	// Skip generator's magic number
	mCurrentInstruction++;

	// Get bound for ids
	mBound = mSource[mCurrentInstruction++];

	mCurrentInstruction++;
	mCurrentInstruction++;

	return true;
}

bool ShaderReflection::ParseInstruction()
{
	InstructionInfo Current(mCurrentInstruction, mSource);
	
	switch (Current.Instruction)
	{
	case SpvOpEntryPoint:
	{
		GetEntryPoint(mCurrentInstruction);
		break;
	}
	case SpvOpName:
	{
		GetName(mCurrentInstruction);
		break;
	}
	case SpvOpMemberName:
	{
		GetMemberName(mCurrentInstruction);
		break;
	}
	case SpvOpMemberDecorate:
	{
		GetMemberDecorate(mCurrentInstruction);
		break;
	}
	case SpvOpDecorate:
	{
		GetDecorate(mCurrentInstruction);
		break;
	}
	case SpvOpTypeArray:
	case SpvOpTypePointer:
	case SpvOpTypeMatrix:
	case SpvOpTypeVector:
	case SpvOpTypeFloat:
	case SpvOpTypeInt:
	case SpvOpTypeStruct:
	case SpvOpTypeSampledImage:
	{
		GetType(mCurrentInstruction);
		break;
	}
	case SpvOpConstant:
	{
		GetConstant(mCurrentInstruction);
		break;
	}
	case SpvOpVariable:
	{
		const uint32_t CurrentId = mSource[mCurrentInstruction + 2];
		mVariablesSection[CurrentId] = mCurrentInstruction;

		const uint32_t Type = mSource[mCurrentInstruction + 3];
		Variable CurrentVariable = GetVariable(mCurrentInstruction);

		switch (Type)
		{
		case SpvStorageClassInput:
		{
			mInputs.push_back(GetInput(CurrentVariable));
			break;
		}
		case SpvStorageClassUniformConstant:
		case SpvStorageClassUniform:
		{
			mUniforms.push_back(GetUniform(CurrentVariable));
			break;
		}
		case SpvStorageClassPushConstant:
		{
			mPushConstants.push_back(GetUniform(CurrentVariable));
			break;
		}
		}

		break;
	}
	case SpvOpFunction:
	{
		return false; // Stop parsing when function section appears
	}
	}

	mCurrentInstruction += Current.Bytes;

	return mCurrentInstruction < mSource.size();
}

void ShaderReflection::GetEntryPoint(uint32_t InstructionIndex)
{
	const uint32_t Id = mSource[mCurrentInstruction + 2];
	const uint32_t ExecModel = mSource[mCurrentInstruction + 1];

	switch (ExecModel)
	{
	case SpvExecutionModelVertex:
	{
		mType = ShaderType::VERTEX;
		break;
	}
	case SpvExecutionModelFragment:
	{
		mType = ShaderType::FRAGMENT;
		break;
	}
	case SpvExecutionModelGLCompute:
	{
		mType = ShaderType::COMPUTE;
		break;
	}
	}

	mEntryPoint = reinterpret_cast<const char*>(&mSource[mCurrentInstruction + 3]);
}

void ShaderReflection::GetName(uint32_t InstructionIndex)
{
	const uint32_t Id = mSource[InstructionIndex + 1];
	const char* NamePtr = reinterpret_cast<const char*>(&mSource[InstructionIndex + 2]);
	mNames[Id] = NamePtr;
}

void ShaderReflection::GetMemberName(uint32_t InstructionIndex)
{
	const uint32_t Id = mSource[InstructionIndex + 1];
	const uint32_t MemberIndex = mSource[InstructionIndex + 2];
	const char* NamePtr = reinterpret_cast<const char*>(&mSource[InstructionIndex + 3]);
	mMemberNames[Id][MemberIndex] = NamePtr;
}

void ShaderReflection::GetDecorate(uint32_t InstructionIndex)
{
	const uint32_t Id = mSource[InstructionIndex + 1];
	mDecorateSection[Id].push_back(InstructionIndex);
}

void ShaderReflection::GetMemberDecorate(uint32_t InstructionIndex)
{
	const uint32_t Id = mSource[InstructionIndex + 1];
	mMemberDecorateSection[Id].push_back(InstructionIndex);
}

void ShaderReflection::GetType(uint32_t InstructionIndex)
{
	const uint32_t CurrentId = mSource[mCurrentInstruction + 1];
	mVariablesSection[CurrentId] = mCurrentInstruction;
}

void ShaderReflection::GetConstant(uint32_t InstructionIndex)
{
	const uint32_t CurrentId = mSource[mCurrentInstruction + 2];
	mVariablesSection[CurrentId] = mCurrentInstruction;
}

Input ShaderReflection::GetInput(const Variable& InputVariablex)
{
	Input Result = {};
	Result.Format = InputVariablex.Type;
	Result.Name = mNames[InputVariablex.Id];

	const auto DecorateList = mDecorateSection[InputVariablex.Id];

	for (auto& Decorate : DecorateList)
	{
		const uint32_t DecorateType = mSource[Decorate + 2];

		switch (DecorateType)
		{
		case SpvDecorationLocation:
		{
			const uint32_t Location = mSource[Decorate + 3];
			Result.Location = Location;
			break;
		}
		case SpvDecorationDescriptorSet:
		{
			const uint32_t Set = mSource[Decorate + 3];
			Result.Set = Set;
			break;
		}
		}

	}

	return Result;
}

Uniform ShaderReflection::GetUniform(const Variable& UniformVariable)
{
	Uniform Result = {};
	Result.Name = mNames[UniformVariable.Id];
	Result.Format = UniformVariable.Type;
	Result.Size = UniformVariable.Size;

	const auto DecorateList = mDecorateSection[UniformVariable.Id];

	for (auto& Decorate : DecorateList)
	{
		const uint32_t DecorateType = mSource[Decorate + 2];

		switch (DecorateType)
		{
		case SpvDecorationBinding:
		{
			const uint32_t Binding = mSource[Decorate + 3];
			Result.Binding = Binding;
			break;
		}
		case SpvDecorationDescriptorSet:
		{
			const uint32_t Set = mSource[Decorate + 3];
			Result.Set = Set;
			break;
		}
		}
	}

	if (Result.Format == VariableType::STRUCTURE)
	{

		const uint32_t TypePtrInstruction = mVariablesSection[UniformVariable.Id];
		uint32_t NextId = mSource[TypePtrInstruction + 1];
		const uint32_t StructureInstruction = mVariablesSection[NextId];
		NextId = mSource[StructureInstruction + 3];

		uint32_t MemberIndex = 0;
		for (auto& Member : UniformVariable.Members)
		{
			Result.Members.push_back(ProcessUniformMember(Member, NextId, MemberIndex++));
		}

	}

	return Result;
}

Variable ShaderReflection::GetVariable(uint32_t InstructionIndex)
{
	const uint32_t ParentId = mSource[InstructionIndex + 1];
	const uint32_t VariableId = mSource[InstructionIndex + 2];
	const uint32_t ParentInstructionIndex = mVariablesSection[ParentId];

	Variable Result = {};
	Result.Id = VariableId;

	InstructionInfo Current(ParentInstructionIndex, mSource);

	// Skip type pointer
	uint32_t NextId = mSource[ParentInstructionIndex + 1];
	if (Current.Instruction == SpvOpTypePointer)
	{
		const uint32_t PointerInstructionIndex = mVariablesSection[NextId];
		NextId = mSource[PointerInstructionIndex + 3];
	}

	// If type is an array then get its size and skip this instruction
	uint32_t NextInstruction = mVariablesSection[NextId];
	Current = InstructionInfo{ NextInstruction, mSource };

	if (Current.Instruction == SpvOpTypeArray)
	{
		const uint32_t ArrayInstructionIndex = mVariablesSection[NextId];
		const uint32_t SizeId = mSource[ArrayInstructionIndex + 3];

		const uint32_t SizeInstruction = mVariablesSection[SizeId];

		InstructionInfo ConstantSize(SizeInstruction, mSource);

		if (ConstantSize.Instruction != SpvOpConstant)
		{
			assert(false); // Array only supports constant size
		}

		const uint32_t Size = mSource[SizeInstruction + 3];
		
		NextId = mSource[ArrayInstructionIndex + 2];
		Result.Size = Size;
	}

	Result.Type = GetVariableType(NextId);
	
	// Handle structure members
	if (Result.Type == VariableType::STRUCTURE)
	{
		const uint32_t Instruction = mVariablesSection[NextId];
		InstructionInfo StructureInstruction(Instruction, mSource);

		for (uint32_t i = 2; i < StructureInstruction.Bytes; ++i)
		{
			const uint32_t Id = mSource[Instruction + i];
			const uint32_t ChildInstruction = mVariablesSection[Id];

			Variable Member = GetVariable(ChildInstruction);

			Result.Members.push_back(Member);
		}

	}

	return Result;
}


UniformMember ShaderReflection::ProcessUniformMember(const Variable& MemberVariable, uint32_t ParentId, uint32_t Index)
{
	UniformMember Result = {};
	Result.Format = MemberVariable.Type;
	Result.Name = mMemberNames[ParentId][Index];
	Result.Size = MemberVariable.Size;

	const auto MemberDecorateList = mMemberDecorateSection[ParentId];

	for (auto& MemberDecorate : MemberDecorateList)
	{
		const uint32_t MemberIndex = mSource[MemberDecorate + 2];
		if(MemberIndex != Index) { continue; }

		const uint32_t MemberDecorateType = mSource[MemberDecorate + 3];

		switch (MemberDecorateType)
		{
		case SpvDecorationOffset:
		{
			const uint32_t MemberOffset = mSource[MemberDecorate + 4];

			Result.Offset = MemberOffset;
			break;
		}
		}

	}

	uint32_t MemberIndex = 0;
	for (auto& Member : MemberVariable.Members)
	{
		Result.Members.push_back(ProcessUniformMember(Member, MemberVariable.Id, MemberIndex++));
	}

	return Result;
}

VariableType ShaderReflection::GetVariableType(uint32_t Id)
{
	const uint32_t Instruction = mVariablesSection[Id];

	InstructionInfo Current(Instruction, mSource);

	switch (Current.Instruction)
	{
	case SpvOpTypeFloat:
		return VariableType::FLOAT;
	case SpvOpTypeInt:
		return VariableType::INT;
	case SpvOpTypeVector:
		return GetTypeOfVector(Id);
	case SpvOpTypeMatrix:
		return GetTypeOfMatrix(Id);
	case SpvOpTypeStruct:
		return VariableType::STRUCTURE;
	case SpvOpTypeSampledImage:
		return VariableType::SAMPLER;
	}

	return VariableType::MAX;
}

VariableType ShaderReflection::GetTypeOfVector(uint32_t Id)
{
	const uint32_t Instruction = mVariablesSection[Id];

	const uint32_t VectorSize = mSource[Instruction + 3];

	const uint32_t NextId = mSource[Instruction + 2];
	const uint32_t NextInstruction = mVariablesSection[NextId];

	InstructionInfo Vector(NextInstruction, mSource);

	switch (Vector.Instruction)
	{
	case SpvOpTypeFloat:
	{
		if (VectorSize == 2)
			return VariableType::FLOAT2;
		else if (VectorSize == 3)
			return VariableType::FLOAT3;
		else if (VectorSize == 4)
			return VariableType::FLOAT4;
		else
			return VariableType::MAX;

		break;
	}
	case SpvOpTypeInt:
	{
		if (VectorSize == 2)
			return VariableType::INT2;
		else if (VectorSize == 3)
			return VariableType::INT3;
		else if (VectorSize == 4)
			return VariableType::INT4;
		else
			return VariableType::MAX;

		break;
	}
	}
	return VariableType::MAX;
}

VariableType ShaderReflection::GetTypeOfMatrix(uint32_t Id)
{
	const uint32_t Instruction = mVariablesSection[Id];

	const uint32_t MatrixSize = mSource[Instruction + 3];

	const uint32_t NextId = mSource[Instruction + 2];
	
	VariableType VectorType = GetTypeOfVector(NextId);

	if (VectorType == VariableType::FLOAT2 && MatrixSize == 2)
		return VariableType::MAT2x2;
	else if (VectorType == VariableType::FLOAT3 && MatrixSize == 3)
		return VariableType::MAT3x3;
	else if (VectorType == VariableType::FLOAT4 && MatrixSize == 4)
		return VariableType::MAT4x4;
			
	return VariableType::MAX;
}

