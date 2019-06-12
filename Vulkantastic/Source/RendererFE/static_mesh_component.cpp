#include "static_mesh_component.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "surface_material.h"
#include "static_mesh.h"

StaticMeshComponent::StaticMeshComponent(const std::string& Name)
{
	SetMesh(Name);
}

StaticMeshComponent::StaticMeshComponent(const StaticMeshComponent& Mesh)
{
	mMesh = std::make_unique<StaticMeshHandle>(Mesh.mMesh->GetStaticMeshName());

	mPosition = Mesh.mPosition;
	mRotation = Mesh.mRotation;
	mScale = Mesh.mScale;
}

StaticMeshComponent::~StaticMeshComponent()
{

}

void StaticMeshComponent::SetMesh(const std::string& Name)
{
	mMesh = std::make_unique<StaticMeshHandle>(Name);
}


glm::mat4 StaticMeshComponent::GetTransform()
{
	const glm::mat4 ScaleMat = glm::scale(glm::mat4(1), mScale);
	const glm::mat4 RotMat = glm::toMat4(mRotation);
	const glm::mat4 TranslationMat = glm::translate(glm::mat4(1), mPosition);

	return TranslationMat * RotMat * ScaleMat;
}

void StaticMeshComponent::SetRotation(float X, float Y, float Z)
{
	mRotation = glm::quat({ glm::degrees(X), glm::degrees(Y), glm::degrees(Z) });
}

void StaticMeshComponent::SetRotation(const glm::vec3& Dir, float Angle)
{
	mRotation = glm::angleAxis(glm::degrees(Angle), Dir);
}
