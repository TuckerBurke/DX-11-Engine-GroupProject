#include "Transform.h"


using namespace DirectX;

Transform::Transform()
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	localScale = XMFLOAT3(1,1,1);
	worldPosition = XMFLOAT3(0,0,0);
	rotation = XMFLOAT3(0,0,0);
	quaternionRotation = XMFLOAT4(0,0,0,1);
}

void Transform::SetPosition(float x, float y, float z)
{
	worldPosition = XMFLOAT3(x, y, z);
	MarkAsDirty();
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation = XMFLOAT3(pitch, yaw, roll);
	XMStoreFloat4(&quaternionRotation, XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
	MarkAsDirty();
}

void Transform::SetScale(float x, float y, float z)
{
	localScale = XMFLOAT3(x, y, z);
	MarkAsDirty();
}

DirectX::XMFLOAT3 Transform::GetPosition() const
{
	return worldPosition;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll() const
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale() const
{
	return localScale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	if(isDirty) 
	{
		CalculateWorldMatrix();
		isDirty = false;
	}
	return worldMatrix;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	worldPosition.x += x;
	worldPosition.y += y;
	worldPosition.z += z;

	MarkAsDirty();
}

void Transform::MoveRelative(float x, float y, float z)
{
	// rotate the direction by the orientation of the object
	DirectX::XMVECTOR translation = DirectX::XMVectorSet(x, y, z, 0);
	DirectX::XMVECTOR rot = DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&rotation));
	
	DirectX::XMVECTOR dir = DirectX::XMVector3Rotate(translation, rot);

	XMStoreFloat3(&worldPosition, XMLoadFloat3(&worldPosition) + dir);

	MarkAsDirty();
}

void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;

	MarkAsDirty();
}

void Transform::Scale(float x, float y, float z)
{
	localScale.x *= x;
	localScale.y *= y;
	localScale.z *= z;

	MarkAsDirty();
}

float Transform::DistanceSquaredTo(DirectX::XMFLOAT3 position)
{
	DirectX::XMVECTOR vec1 = DirectX::XMLoadFloat3(&position);
	DirectX::XMVECTOR vec2 = DirectX::XMLoadFloat3(&this->worldPosition);
	DirectX::XMVECTOR vec3 = DirectX::XMVectorSubtract(vec1, vec2);
	float distSqrd;
	DirectX::XMStoreFloat(&distSqrd, DirectX::XMVector3LengthSq(vec3));
	return distSqrd;
}

void Transform::CalculateWorldMatrix()
{
	//create the translation matrix
	DirectX::XMMATRIX tranM = DirectX::XMMatrixTranslation(worldPosition.x, worldPosition.y, worldPosition.z);

	// create the rotation matrix
	DirectX::XMMATRIX rotM = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	//create the scale matrix
	DirectX::XMMATRIX scaleM = DirectX::XMMatrixScaling(localScale.x, localScale.y, localScale.z);

	DirectX::XMMATRIX world = scaleM * rotM * tranM; // is this in simd?

	XMStoreFloat4x4(&worldMatrix, world);
}
