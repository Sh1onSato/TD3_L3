#define NOMINMAX

#include "Player.h"
#include <cassert>
#include <numbers>
#include <algorithm>
#include "Math.h"
#include "MapChipField.h"

/**
 * @brief 初期化
 */
void Player::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const Vector3& position) { 
	assert(model);
	
	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	// ブロックの真上に乗るように Y を調整 (ブロック高さ1.0の半分 + プレイヤー高さ0.8の半分)
	worldTransform_.translation_ = position;
	worldTransform_.translation_.y = 0.9f; 
	
	// 初期方向を前（Z軸プラス）に向ける
	worldTransform_.rotation_.y = 0.0f;
}

/**
 * @brief 更新
 */
void Player::Update() { 
	InputMove();
	
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_; 
	CheckMapCollision(collisionMapInfo);

	worldTransform_.translation_ += collisionMapInfo.move;

	UpdateOnWall(collisionMapInfo);

	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);
		worldTransform_.rotation_.y = EaseInOut(1.0f - (turnTimer_ / kTimeTurn), turnFirstRotationY_, worldTransform_.rotation_.y);
	}

	WorldTransformUpdate(worldTransform_);
}

/**
 * @brief 描画
 */
void Player::Draw() { 
	model_->Draw(worldTransform_, *camera_, textureHandle_); 
}

/**
 * @brief 移動入力の処理
 */
void Player::InputMove() {
	Vector3 acceleration = {};
	
	if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		acceleration.x += kAcceleration;
	} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
		acceleration.x -= kAcceleration;
	}

	if (Input::GetInstance()->PushKey(DIK_UP)) {
		acceleration.z += kAcceleration;
	} else if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		acceleration.z -= kAcceleration;
	}

	if (acceleration.x != 0.0f) velocity_.x += acceleration.x;
	else velocity_.x *= (1.0f - kAttenuation);

	if (acceleration.z != 0.0f) velocity_.z += acceleration.z;
	else velocity_.z *= (1.0f - kAttenuation);

	velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	velocity_.z = std::clamp(velocity_.z, -kLimitRunSpeed, kLimitRunSpeed);

	if (std::abs(velocity_.x) <= 0.001f) velocity_.x = 0.0f;
	if (std::abs(velocity_.z) <= 0.001f) velocity_.z = 0.0f;

	if (acceleration.x != 0.0f || acceleration.z != 0.0f) {
		worldTransform_.rotation_.y = std::atan2(acceleration.x, acceleration.z);
	}
}

/**
 * @brief マップとの当たり判定（空白を壁として扱う）
 */
void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionRight(info); 
	CheckMapCollisionLeft(info);  
	CheckMapCollisionUp(info);    
	CheckMapCollisionDown(info);  
}

void Player::CheckMapCollisionRight(CollisionMapInfo& info) { 
	if (info.move.x <= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, 0, 0);
	Vector3 rightForward = nextPos + Vector3(kWidth / 2.0f, 0, kHeight / 2.0f - kBlank * 2.0f);
	Vector3 rightBackward = nextPos + Vector3(kWidth / 2.0f, 0, -kHeight / 2.0f + kBlank * 2.0f);

	MapChipField::IndexSet indexRF = mapChipField_->GetMapChipIndexSetByPosition(rightForward);
	MapChipField::IndexSet indexRB = mapChipField_->GetMapChipIndexSetByPosition(rightBackward);

	// ブロックがない（kBlank）ならヒット（進めない）
	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexRF.xIndex, indexRF.yIndex) == MapChipType::kBlank) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexRB.xIndex, indexRB.yIndex) == MapChipType::kBlank);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexRF.xIndex, indexRF.yIndex);
		info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) { 
	if (info.move.x >= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, 0, 0);
	Vector3 leftForward = nextPos + Vector3(-kWidth / 2.0f, 0, kHeight / 2.0f - kBlank * 2.0f);
	Vector3 leftBackward = nextPos + Vector3(-kWidth / 2.0f, 0, -kHeight / 2.0f + kBlank * 2.0f);

	MapChipField::IndexSet indexLF = mapChipField_->GetMapChipIndexSetByPosition(leftForward);
	MapChipField::IndexSet indexLB = mapChipField_->GetMapChipIndexSetByPosition(leftBackward);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexLF.xIndex, indexLF.yIndex) == MapChipType::kBlank) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexLB.xIndex, indexLB.yIndex) == MapChipType::kBlank);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexLF.xIndex, indexLF.yIndex);
		info.move.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	if (info.move.z <= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, 0, info.move.z);
	Vector3 forwardLeft = nextPos + Vector3(-kWidth / 2.0f + kBlank * 2.0f, 0, kHeight / 2.0f);
	Vector3 forwardRight = nextPos + Vector3(kWidth / 2.0f - kBlank * 2.0f, 0, kHeight / 2.0f);

	MapChipField::IndexSet indexFL = mapChipField_->GetMapChipIndexSetByPosition(forwardLeft);
	MapChipField::IndexSet indexFR = mapChipField_->GetMapChipIndexSetByPosition(forwardRight);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexFL.xIndex, indexFL.yIndex) == MapChipType::kBlank) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexFR.xIndex, indexFR.yIndex) == MapChipType::kBlank);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexFL.xIndex, indexFL.yIndex);
		info.move.z = std::max(0.0f, rect.bottom - worldTransform_.translation_.z - (kHeight / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.move.z >= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, 0, info.move.z);
	Vector3 backwardLeft = nextPos + Vector3(-kWidth / 2.0f + kBlank * 2.0f, 0, -kHeight / 2.0f);
	Vector3 backwardRight = nextPos + Vector3(kWidth / 2.0f - kBlank * 2.0f, 0, -kHeight / 2.0f);

	MapChipField::IndexSet indexBL = mapChipField_->GetMapChipIndexSetByPosition(backwardLeft);
	MapChipField::IndexSet indexBR = mapChipField_->GetMapChipIndexSetByPosition(backwardRight);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexBL.xIndex, indexBL.yIndex) == MapChipType::kBlank) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexBR.xIndex, indexBR.yIndex) == MapChipType::kBlank);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexBL.xIndex, indexBL.yIndex);
		info.move.z = std::min(0.0f, rect.top - worldTransform_.translation_.z + (kHeight / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::UpdateOnGround(const CollisionMapInfo& info) {
	onGround_ = true;
	(void)info;
}

void Player::UpdateOnWall(const CollisionMapInfo& info) {
	if (info.hitWall) {
		if (std::abs(info.move.x) < 0.001f) velocity_.x = 0.0f;
		if (std::abs(info.move.z) < 0.001f) velocity_.z = 0.0f;
	}
}

float Player::EaseInOut(float t, float start, float end) {
	t = std::clamp(t, 0.0f, 1.0f);
	float t_eased = t * t * (3.0f - 2.0f * t);
	return start + (end - start) * t_eased;
}

Vector3 Player::GetWorldPosition() {
	return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]};
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	return {
		{worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f},
		{worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f}
	};
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	isDead_ = true;
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[] = {
	    {+kWidth / 2.0f, 0, -kHeight / 2.0f}, // 右下
	    {-kWidth / 2.0f, 0, -kHeight / 2.0f}, // 左下
	    {+kWidth / 2.0f, 0, +kHeight / 2.0f}, // 右上
	    {-kWidth / 2.0f, 0, +kHeight / 2.0f}  // 左上
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}
