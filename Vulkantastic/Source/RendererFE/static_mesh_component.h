#pragma once

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class StaticMeshHandle;
class StaticMesh;

class StaticMeshComponent
{
	
public:
	StaticMeshComponent() = default;
	StaticMeshComponent(const std::string& Name);
	StaticMeshComponent(const StaticMeshComponent& Mesh);

	~StaticMeshComponent();

	void SetMesh(const std::string& Name);
	inline StaticMeshHandle* GetMeshHandle() const { return mMesh.get(); }

	glm::mat4 GetTransform();

	inline void SetPosition(const glm::vec3& Pos) { mPosition = Pos; }
	inline void SetScale(const glm::vec3& Scale) { mScale = Scale; }
	void SetRotation(float X, float Y, float Z);
	void SetRotation(const glm::vec3& Dir, float Angle);

	inline glm::vec3 GetPosition(const glm::vec3& Pos) const { return mPosition; }
	inline glm::quat GetRotation(const glm::quat& Rot) const { return mRotation; }
	inline glm::vec3 GetScale(const glm::vec3& Scale) const { return mScale; }

private:
	glm::vec3 mPosition = { 0,0,0 };
	glm::quat mRotation = {};
	glm::vec3 mScale = { 1,1,1 };

	std::unique_ptr<StaticMeshHandle> mMesh;

};
