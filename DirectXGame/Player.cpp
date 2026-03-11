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
	worldTransform_.translation_ = position;
	
	// 初期方向を右に向ける（90度）
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

/**
 * @brief 更新
 */
void Player::Update() { 
	// 1. キー入力による移動速度の計算
	InputMove();
	
	// 2. マップとの当たり判定（移動量を補正する）
	CollisionMapInfo collisionMapInfo = {};
	collisionMapInfo.move = velocity_; 
	CheckMapCollision(collisionMapInfo);

	// 3. 補正された移動量を座標に反映
	worldTransform_.translation_ += collisionMapInfo.move;

	// 4. 特殊な状態の更新
	if (collisionMapInfo.ceiling) {
		velocity_.y = 0; // 天井にぶつかったら上昇を止める
	}
	UpdateOnWall(collisionMapInfo); // 壁接触時の処理
	UpdateOnGround(collisionMapInfo); // 接地判定

	// 5. 旋回（方向転換）のアニメーション
	if (turnTimer_ > 0.0f) {
		turnTimer_ = std::max(turnTimer_ - (1.0f / 60.0f), 0.0f);

		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f, 
			std::numbers::pi_v<float> * 3.0f / 2.0f
		};
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		worldTransform_.rotation_.y = EaseInOut(1.0f - (turnTimer_ / kTimeTurn), turnFirstRotationY_, destinationRotationY);
	}

	// 6. 行列の更新
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
	if (onGround_) {
		if (Input::GetInstance()->PushKey(DIK_RIGHT) || Input::GetInstance()->PushKey(DIK_LEFT)) {
			Vector3 acceleration = {};
			
			if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
				if (velocity_.x < 0.0f) velocity_.x *= (1.0f - kAttenuation);
				acceleration.x += kAcceleration;

				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (Input::GetInstance()->PushKey(DIK_LEFT)) {
				if (velocity_.x > 0.0f) velocity_.x *= (1.0f - kAttenuation);
				acceleration.x -= kAcceleration;

				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotation_.y;
					turnTimer_ = kTimeTurn;
				}
			}
			
			velocity_.x += acceleration.x;
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}

		if (std::abs(velocity_.x) <= 0.0001f) velocity_.x = 0.0f;

		if (Input::GetInstance()->PushKey(DIK_UP)) {
			velocity_.y += kJumpAcceleration / 60.0f;
			onGround_ = false;
		}

	} else {
		velocity_.y -= kGravityAcceleration / 60.0f;
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

/**
 * @brief マップとの当たり判定
 */
void Player::CheckMapCollision(CollisionMapInfo& info) {
	// 1. 横移動の判定（縦の座標は今のまま、横にだけ動かしてチェック）
	CheckMapCollisionRight(info); 
	CheckMapCollisionLeft(info);  

	// 2. 縦移動の判定（横移動で補正された後の位置から、縦に動かしてチェック）
	CheckMapCollisionUp(info);    
	CheckMapCollisionDown(info);  
}

// --- 横の壁判定（上下を少し短くして判定することで、床や天井に引っかかるのを防ぐ） ---

void Player::CheckMapCollisionRight(CollisionMapInfo& info) { 
	if (info.move.x <= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, 0, 0);
	Vector3 rightTop = nextPos + Vector3(kWidth / 2.0f, kHeight / 2.0f - kBlank * 2.0f, 0);
	Vector3 rightBottom = nextPos + Vector3(kWidth / 2.0f, -kHeight / 2.0f + kBlank * 2.0f, 0);

	MapChipField::IndexSet indexRT = mapChipField_->GetMapChipIndexSetByPosition(rightTop);
	MapChipField::IndexSet indexRB = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexRT.xIndex, indexRT.yIndex) == MapChipType::kBlock) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexRB.xIndex, indexRB.yIndex) == MapChipType::kBlock);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexRT.xIndex, indexRT.yIndex);
		info.move.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

void Player::CheckMapCollisionLeft(CollisionMapInfo& info) { 
	if (info.move.x >= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, 0, 0);
	Vector3 leftTop = nextPos + Vector3(-kWidth / 2.0f, kHeight / 2.0f - kBlank * 2.0f, 0);
	Vector3 leftBottom = nextPos + Vector3(-kWidth / 2.0f, -kHeight / 2.0f + kBlank * 2.0f, 0);

	MapChipField::IndexSet indexLT = mapChipField_->GetMapChipIndexSetByPosition(leftTop);
	MapChipField::IndexSet indexLB = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexLT.xIndex, indexLT.yIndex) == MapChipType::kBlock) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexLB.xIndex, indexLB.yIndex) == MapChipType::kBlock);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexLT.xIndex, indexLT.yIndex);
		info.move.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}
}

// --- 縦の天井・床判定（左右を少し細くして判定することで、壁の角に引っかかるのを防ぐ） ---

void Player::CheckMapCollisionUp(CollisionMapInfo& info) {
	if (info.move.y <= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, info.move.y, 0);
	Vector3 leftTop = nextPos + Vector3(-kWidth / 2.0f + kBlank * 2.0f, kHeight / 2.0f, 0);
	Vector3 rightTop = nextPos + Vector3(kWidth / 2.0f - kBlank * 2.0f, kHeight / 2.0f, 0);

	MapChipField::IndexSet indexLT = mapChipField_->GetMapChipIndexSetByPosition(leftTop);
	MapChipField::IndexSet indexRT = mapChipField_->GetMapChipIndexSetByPosition(rightTop);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexLT.xIndex, indexLT.yIndex) == MapChipType::kBlock) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexRT.xIndex, indexRT.yIndex) == MapChipType::kBlock);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexLT.xIndex, indexLT.yIndex);
		info.move.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - (kHeight / 2.0f + kBlank));
		info.ceiling = true;
	}
}

void Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.move.y >= 0) return;

	Vector3 nextPos = worldTransform_.translation_ + Vector3(info.move.x, info.move.y, 0);
	Vector3 leftBottom = nextPos + Vector3(-kWidth / 2.0f + kBlank * 2.0f, -kHeight / 2.0f, 0);
	Vector3 rightBottom = nextPos + Vector3(kWidth / 2.0f - kBlank * 2.0f, -kHeight / 2.0f, 0);

	MapChipField::IndexSet indexLB = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
	MapChipField::IndexSet indexRB = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	bool hit = (mapChipField_->GetMapChipTypeByIndex(indexLB.xIndex, indexLB.yIndex) == MapChipType::kBlock) ||
	           (mapChipField_->GetMapChipTypeByIndex(indexRB.xIndex, indexRB.yIndex) == MapChipType::kBlock);

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexLB.xIndex, indexLB.yIndex);
		info.move.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + (kHeight / 2.0f + kBlank));
		info.landing = true;
	}
}

/**
 * @brief キャラクターの中心座標から指定した角の座標を計算（AABB用などの汎用）
 */
Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0}, // 右下
	    {-kWidth / 2.0f, -kHeight / 2.0f, 0}, // 左下
	    {+kWidth / 2.0f, +kHeight / 2.0f, 0}, // 右上
	    {-kWidth / 2.0f, +kHeight / 2.0f, 0}  // 左上
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

/**
 * @brief 接地状態の更新
 */
void Player::UpdateOnGround(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false; 
		} else {
			Vector3 pos = worldTransform_.translation_ + Vector3(info.move.x, info.move.y, 0);
			Vector3 leftFoot = pos + Vector3(-kWidth / 2.0f + kBlank * 2.0f, -kHeight / 2.0f - kGroundSearchHeight, 0);
			Vector3 rightFoot = pos + Vector3(kWidth / 2.0f - kBlank * 2.0f, -kHeight / 2.0f - kGroundSearchHeight, 0);

			MapChipField::IndexSet indexLF = mapChipField_->GetMapChipIndexSetByPosition(leftFoot);
			MapChipField::IndexSet indexRF = mapChipField_->GetMapChipIndexSetByPosition(rightFoot);

			bool hit = (mapChipField_->GetMapChipTypeByIndex(indexLF.xIndex, indexLF.yIndex) == MapChipType::kBlock) ||
			           (mapChipField_->GetMapChipTypeByIndex(indexRF.xIndex, indexRF.yIndex) == MapChipType::kBlock);
			
			if (!hit) onGround_ = false;
		}
	} else if (info.landing) {
		onGround_ = true;
		velocity_.x *= (1.0f - kAttenuationLanding);
		velocity_.y = 0.0f;
	}
}

void Player::UpdateOnWall(const CollisionMapInfo& info) {
	if (info.hitWall) {
		velocity_.x = 0.0f;
	}
}

/**
 * @brief 滑らかな補間（イージング）
 */
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
