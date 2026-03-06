#pragma once
#include <array>
#include <numbers>
#include <algorithm>
#include "KamataEngine.h"
#include "Math.h"
class DeathParticles {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	
	void Update();
	
	void Draw();

	bool IsFinished() { return isFinished_; }

private:
	static inline const uint32_t kNumParticles = 8;

	std::array<WorldTransform, kNumParticles> worldTransforms_;

	Model* model_ = nullptr;

	Camera* camera_ = nullptr;

	static inline const float kDuration_ = 2.0f;

	static inline const float kSpeed_ = 0.05f;

	static inline const float kAngleUnit_ = 2.0f * std::numbers::pi_v<float> / kNumParticles;

	bool isFinished_ = false;

	float counter_ = 0.0f;

	ObjectColor objectColor_;

	Vector4 color_;
};
