#include "Camera.h"

Camera::Camera(DirectX::XMFLOAT3 initPosition /*= DirectX::XMFLOAT3(0,0,0)*/, DirectX::XMFLOAT3 initRotation /*= DirectX::XMFLOAT3(0,0,0)*/, float aspectRatio /*= 1920.f / 1080.f*/, float fieldOfView /*= 90.f*/, float nearClip /*= 0.01f*/, float farClip /*= 10.f*/, float movementSpeed /*= 5.f*/, float mouseLookSpeed /*= 1.f */)
{
	transform.SetPosition(initPosition.x, initPosition.y, initPosition.z);
	transform.SetRotation(initRotation.x, initRotation.y, initRotation.z);

	fov = fieldOfView;
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->mouseLookSpeed = mouseLookSpeed;
	this->movementSpeed = movementSpeed;

	mousePos = {};

	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	 DirectX::XMStoreFloat4x4
	 (
		 &projMatrix, 
		 DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, aspectRatio, nearClip, 100.0f)
	 );
}

void Camera::UpdateViewMatrix()
{
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0,0,1,0);
	DirectX::XMVECTOR rotQuat = DirectX::XMQuaternionRotationRollPitchYawFromVector
	(
		DirectX::XMLoadFloat3
		(
			&transform.GetPitchYawRoll()
		)
	);

	forward = DirectX::XMVector3Rotate(forward, rotQuat);

	DirectX::XMMATRIX lookToMatrix = DirectX::XMMatrixLookToLH
	(
		DirectX::XMLoadFloat3(&transform.GetPosition()),
		forward,
		DirectX::XMVectorSet(0, 1, 0, 0)
	);

	DirectX::XMStoreFloat4x4(&viewMatrix, lookToMatrix);
}

void Camera::DestroySelf()
{
	// @todo. Also consider making a class wrapper so that not the camera class inherits the Player interface
}
