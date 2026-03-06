#pragma once

#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;
class Player;

class Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	void Update();

	void Draw();

	AABB GetAABB();

	Vector3 GetWorldPosition();

	void OnCollision(const Player* player);

private:
	WorldTransform worldTransform_;

	Model* model_ = nullptr;

	Camera* camera_ = nullptr;

	static inline const float kWalkSpeed = 0.02f;

	Vector3 velocity_ = {};

	static inline const float kWalkMotionAngleStart = 0.0f;

	static inline const float kWalkMotionAngleEnd = 30.0f;

	static inline const float kWalkMotionTime = 1.0f;

	float walkTimer = 0.0f;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;
};
