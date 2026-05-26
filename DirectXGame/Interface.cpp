#include "Interface.h"

void Interface::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	assert(model);
	model_ = model;
	camera_ = camera;

	// 配列のメモリを確保
	for (int i = 0; i < kMaxAttack; i++) {
		Transforms_[i] = new KamataEngine::WorldTransform();
		Transforms_[i]->Initialize();
		Transforms_[i]->scale_ = {0.25f, 0.25f, 0.25f};
	}
}

void Interface::Update(int currentAttack) {
	int index = currentAttack;
	if (index >= kMaxAttack)
		index = kMaxAttack - 1;
	if (index < 0)
		index = 0;

	Vector3 basePosition;
	basePosition.x = camera_->translation_.x + cameraOffset_.x;
	basePosition.y = camera_->translation_.y + cameraOffset_.y;
	basePosition.z = camera_->translation_.z + cameraOffset_.z;

	float intervalX = 0.3f;

	for (int i = 0; i < kMaxAttack; i++) {
		Transforms_[i]->translation_ = basePosition;
		Transforms_[i]->translation_.x += (float)i * intervalX;

		Transforms_[i]->rotation_.x = 0.4f;


		WorldTransformUpdate(*Transforms_[i]);
	}

	targetX_ = Transforms_[index]->translation_.x;
	targetY_ = Transforms_[index]->translation_.y;
}

void Interface::Draw(int remainingMoves) {
	if (remainingMoves <= 0)
		return;

	for (int i = 0; i < remainingMoves; i++) {
		model_->Draw(*Transforms_[i], *camera_);
	}
}