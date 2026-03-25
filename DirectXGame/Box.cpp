#include "Box.h"
#include "Math.h"

using namespace KamataEngine;

void Box::Initialize(Model* model, KamataEngine::Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;
	position_ = position;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	alive_ = true;
}

void Box::Update() {
	if (!alive_) {
		return;
	}

	// 行列の更新 (Enemy.cpp と同じ計算式)
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// 定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Box::Draw() {
	if (alive_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

void Box::OnCollision() {
	if (alive_) {
		alive_ = false;
		breakCount++; // 壊した数を加算
	}
}

Vector3 Box::GetWorldPosition() {
	Vector3 worldPos;
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];
	return worldPos;
}

AABB Box::GetAABB() {
	if (!alive_) {
		return {
		    {0, 0, 0},
            {0, 0, 0}
        };
	}

	Vector3 worldPos = GetWorldPosition();
	const float kSize = 1.0f; // 箱のサイズ

	AABB aabb;
	aabb.min = {
	    worldPos.x - kSize / 2.0f,
	    worldPos.y - kSize / 2.0f,
	    worldPos.z - kSize / 2.0f,
	};
	aabb.max = {
	    worldPos.x + kSize / 2.0f,
	    worldPos.y + kSize / 2.0f,
	    worldPos.z + kSize / 2.0f,
	};
	return aabb;
}