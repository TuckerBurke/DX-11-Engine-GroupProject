#pragma once

#include <DirectXMath.h>

class Entity;
class Transform;
class PlayerInterface;

enum class AI_State: unsigned char
{
	DEFAULT = 0x01,
	PATROL_PATH = 0x02,
	ATTACK_PLAYER = 0x04
};

class SimpleAI 
{
	// Color tint constants
	const DirectX::XMFLOAT4 AttackColor = DirectX::XMFLOAT4(1.f, .1f, .1f, .5f);
	const DirectX::XMFLOAT4 PatrolColor = DirectX::XMFLOAT4(.1f, .1f, 1.f, .5f);

public:
	SimpleAI(class PlayerInterface* pPlayer, class Entity** path, class Entity* pSelf);
	~SimpleAI() = default;

	inline void SetState(AI_State pState) {state = pState;}
	virtual void Update(bool playerInLight, float deltaTime);

	class Entity* self = nullptr;

private:
	void ExecutePatrolPath(float deltaTime);
	void ExecuteAttackPlayer(float deltaTime);

	// Updates the internal AI_State based on player distance
	void UpdateState(bool inLight);

	// Helper method for movement operations towards another transform
	void AIMoveTowards(Transform* pTarget, float deltaTime);

	class Entity** targetPath = nullptr;
	class PlayerInterface* player = nullptr;
	
	AI_State state = AI_State::DEFAULT;
	size_t activeRoute;
	size_t maxRouteCount;

	float ghostSpeedBoost;
};