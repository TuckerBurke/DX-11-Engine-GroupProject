#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include "Transform.h"
#include "PlayerInterface.h"

class Camera: public PlayerInterface 
{
public:
	Camera
	(
		DirectX::XMFLOAT3 initPosition = DirectX::XMFLOAT3(0,0,0), 
		DirectX::XMFLOAT3 initRotation = DirectX::XMFLOAT3(0,0,0), 
		float aspectRatio = 1920.f / 1080.f,
		float fieldOfView = 90.f, 
		float nearClip = 0.01f, 
		float farClip = 900.f,
		float movementSpeed = 5.f,
		float mouseLookSpeed = 2.f
	);
	~Camera() = default;

	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();

	inline DirectX::XMFLOAT4X4 GetProjectionMatrix() const { return projMatrix; };
	inline DirectX::XMFLOAT4X4 GetViewMatrix() const { return viewMatrix; };
	inline Transform* GetTransform() override { return &transform; }
	inline float GetSensitivity() const { return mouseLookSpeed; }
	inline float GetMovementSpeed() const { return movementSpeed; }

	virtual void DestroySelf();

private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
	
	POINT mousePos;

	float fov;
	float nearClip;
	float farClip;

	float movementSpeed;
	float mouseLookSpeed;
};
