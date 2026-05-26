#pragma once
#include "KamataEngine.h"
#include "Math.h"

class Interface {
public:
	static const int kMaxAttack = 10;

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);
	void Update(int currentAttack);
	void Draw(int remainingMoves);

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::WorldTransform* Transforms_[kMaxAttack];

	Vector3 cameraOffset_ = {-3.9f, -3.0f, 6.0f}; 

	float targetX_ = 0.0f;
	float targetY_ = 0.0f;
};