#pragma once

#include <DirectXMath.h>

class Transform 
{
public:
	Transform();
	~Transform() = default;

	void SetPosition(float x, float y, float z);
	void SetRotation(float pitch, float yaw, float roll);
	void SetScale(float x, float y, float z);

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetPitchYawRoll() const;
	DirectX::XMFLOAT3 GetScale() const;

	// will recalculate the world matrix if is dirty
	DirectX::XMFLOAT4X4 GetWorldMatrix();

	void MoveAbsolute(float x, float y, float z);
	void MoveRelative(float x,float y,float z);
	void Rotate(float pitch, float yaw, float roll);
	void Scale(float x, float y, float z);

	float DistanceSquaredTo(DirectX::XMFLOAT3 position);

private:

	void MarkAsDirty(){isDirty=true;}
	void CalculateWorldMatrix();

	bool isDirty = false;

	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT3 worldPosition;
	DirectX::XMFLOAT3 localScale;

	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT4 quaternionRotation;
};