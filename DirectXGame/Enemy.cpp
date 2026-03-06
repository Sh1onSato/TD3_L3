#include <cassert>
#include <numbers>
#include "Enemy.h"
#include "Math.h"

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model);

	model_ = model;

	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	velocity_ = {-kWalkSpeed, 0, 0};

	walkTimer = 0.0f;
}

void Enemy::Update() {

	worldTransform_.translation_ += velocity_;

	walkTimer += 1.0f / 60.0f;

	worldTransform_.rotation_.x = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);

	WorldTransformUpdate(worldTransform_);
}

void Enemy::Draw() {

	model_->Draw(worldTransform_, *camera_);
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

Vector3 Enemy::GetWorldPosition() {
	Vector3 worldPos;

	// ワールド行列の平行移動成分を取得（ワールド座標）
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

void Enemy::OnCollision(const Player* player) {
	(void)player;
	//
}
