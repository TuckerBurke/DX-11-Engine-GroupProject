#pragma once
class Transform;

class PlayerInterface
{
public:
	PlayerInterface() = default;
	virtual ~PlayerInterface() = default;

	// Must have a way to access the transform so the AI knows where to chase
	virtual Transform* GetTransform() = 0;

	virtual void DestroySelf() = 0; 
};