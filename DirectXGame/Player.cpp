#define NOMINMAX

#include "Player.h"
#include <cassert>
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

	// マス移動アニメーション
	if (isMoving_) {
		moveTimer_ += 1.0f / 60.0f;
		float t = moveTimer_ / kMoveTime;

		if (t >= 1.0f) {
			// 移動完了：目標マスの中心にスナップ
			worldTransform_.translation_ = moveTargetPosition_;
			isMoving_ = false;
		} else {
			// 線形補間（直線移動）
			worldTransform_.translation_.x = moveStartPosition_.x + (moveTargetPosition_.x - moveStartPosition_.x) * t;
			worldTransform_.translation_.z = moveStartPosition_.z + (moveTargetPosition_.z - moveStartPosition_.z) * t;
		}
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
 * @brief 移動入力の処理（マス単位の移動）
 */
void Player::InputMove() {
	// 移動アニメーション中は新たな入力を受け付けない
	if (isMoving_) return;

	int32_t dx = 0, dz = 0;

	if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		dx = 1;
	} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
		dx = -1;
	} else if (Input::GetInstance()->PushKey(DIK_UP)) {
		dz = -1; // yIndex を -1 する → worldZ = kBlockHeight*(kNumBlockVirtical-1-yIndex) が増加 → 前進(+Z)
	} else if (Input::GetInstance()->PushKey(DIK_DOWN)) {
		dz = 1;  // yIndex を +1 する → worldZ が減少 → 後退(-Z)
	}

	if (dx == 0 && dz == 0) return;

	// 現在のグリッドインデックスを取得
	MapChipField::IndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

	int32_t nextXIndex = static_cast<int32_t>(currentIndex.xIndex) + dx;
	int32_t nextYIndex = static_cast<int32_t>(currentIndex.yIndex) + dz;

	// 範囲外チェック
	if (nextXIndex < 0 || nextYIndex < 0) return;

	// 移動先のマスが通行可能（kBlock）かチェック
	MapChipType nextType = mapChipField_->GetMapChipTypeByIndex(
	    static_cast<uint32_t>(nextXIndex), static_cast<uint32_t>(nextYIndex));
	if (nextType != MapChipType::kBlock) return;

	// 移動アニメーションのセットアップ
	moveStartPosition_ = worldTransform_.translation_;
	moveTargetPosition_ = mapChipField_->GetMapChipPositionByIndex(
	    static_cast<uint32_t>(nextXIndex), static_cast<uint32_t>(nextYIndex));
	moveTargetPosition_.y = worldTransform_.translation_.y; // Y は固定
	isMoving_ = true;
	moveTimer_ = 0.0f;

	// キャラクターを移動方向に向ける
	worldTransform_.rotation_.y = std::atan2(static_cast<float>(dx), static_cast<float>(-dz));
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
