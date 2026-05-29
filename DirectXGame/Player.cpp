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

	for (int i = 0; i < 8; ++i) {
		guideTransforms_[i].Initialize();
	}
	
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

			// 移動回数を減らす
			if (remainingMoves_ > 0) {
				remainingMoves_--;
			}
		} else {
			// EaseInOut で滑らかに補間
			worldTransform_.translation_.x = EaseInOut(t, moveStartPosition_.x, moveTargetPosition_.x);
			worldTransform_.translation_.z = EaseInOut(t, moveStartPosition_.z, moveTargetPosition_.z);
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

/**i
 * @brief 移動入力の処理（マス単位の移動）
 */
void Player::InputMove() {
	//移動アニメーション中は入力を受け付けない
	if (isMoving_) return;

	int32_t dx = 0, dz = 0;

	if (playerIndex_ == 0) {
		// プレイヤー0（シオン）: 矢印キー + SPACE
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
			dx += 1;
		}
		if (Input::GetInstance()->PushKey(DIK_LEFT)) {
			dx -= 1;
		}
		if (Input::GetInstance()->PushKey(DIK_UP)) {
			dz -= 1;
		}
		if (Input::GetInstance()->PushKey(DIK_DOWN)) {
			dz += 1;
		}
	} else {
		// プレイヤー2（ゆか）: WASDキー + Z
		if (Input::GetInstance()->PushKey(DIK_D)) {
			dx += 1;
		}
		if (Input::GetInstance()->PushKey(DIK_A)) {
			dx -= 1;
		}
		if (Input::GetInstance()->PushKey(DIK_W)) {
			dz -= 1;
		}
		if (Input::GetInstance()->PushKey(DIK_S)) {
			dz += 1;
		}
	}

	//斜めの方向処理
	if (dx != 0 || dz != 0) {
		float len = std::sqrt(float(dx * dx + dz * dz));
		moveDirection_.x = dx / len;
		moveDirection_.z = dz / len;

		worldTransform_.rotation_.y = std::atan2(moveDirection_.x, -moveDirection_.z);
	}

	// スライド開始キー（P0: SPACE、P1: Z）
	bool slideKey = (playerIndex_ == 0)
	    ? Input::GetInstance()->TriggerKey(DIK_SPACE)
	    : Input::GetInstance()->TriggerKey(DIK_Z);

	if (slideKey) {
		// 移動回数が残っていないなら移動不可
		if (remainingMoves_ <= 0) return;

		moveStartPosition_ = worldTransform_.translation_;
		moveTargetPosition_ = GetSlideTargetPosition();

		// 動かないなら無視
		if (moveStartPosition_.x == moveTargetPosition_.x && moveStartPosition_.z == moveTargetPosition_.z) {
			return;
		}

		isMoving_ = true;
		moveTimer_ = 0.0f;
	}

}

Vector3 Player::GetSlideTargetPosition() {

	auto current = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

	int x = static_cast<int>(current.xIndex);
	int z = static_cast<int>(current.yIndex); 

	int stepX = static_cast<int>(std::round(moveDirection_.x));
	int stepZ = static_cast<int>(std::round(moveDirection_.z));

	// 方向がない場合は現在の位置を返す
	if (stepX == 0 && stepZ == 0) {
		return worldTransform_.translation_;
	}

	while (true) {
		int nextX = x + stepX;
		int nextZ = z + stepZ;

		// 目的のマスに床があるかチェック (layer 0)
		auto type = mapChipField_->GetMapChipTypeByIndex(nextX, nextZ, 0);
		if (type != MapChipType::kBlock) {
			break;
		}

		// 移動確定（障害物があっても壊して進む仕様なので継続）
		x = nextX;
		z = nextZ;
	}

	Vector3 pos = mapChipField_->GetMapChipPositionByIndex((uint32_t)x, (uint32_t)z);

	pos.y = worldTransform_.translation_.y;

	return pos;
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


std::vector<KamataEngine::WorldTransform*> Player::GetGuideTransforms() {
	std::vector<KamataEngine::WorldTransform*> activeGuides;

	if (isMoving_ || remainingMoves_ <= 0)
		return activeGuides;

	auto current = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
	int x = static_cast<int>(current.xIndex);
	int z = static_cast<int>(current.yIndex);

	float offsetDistance = 1.2f;
	float arrowHeight = worldTransform_.translation_.y + 0.6f;

	// マップの配列を「調べる方向（check）」と、画面上で「矢印を出す方向（disp）」を分ける
	struct DirectionInfo {
		int checkX;
		int checkZ;
		float dispX;
		float dispZ;
		float rotateY; // 矢印の回転角度
	};

	DirectionInfo dirs[] = {
	    {1,  0,  1.0f,  0.0f,  3.1415f * 0.5f  }, //  右
	    {-1, 0,  -1.0f, 0.0f,  -3.1415f * 0.5f }, //  左
	    {0,  -1, 0.0f,  1.0f,  0.0f            }, //  上
	    {0,  1,  0.0f,  -1.0f, 3.1415f         }, //  下

	    {1,  -1, 1.0f,  1.0f,  3.1415f * 0.25f }, // 右上
	    {-1, -1, -1.0f, 1.0f,  -3.1415f * 0.25f}, // 左上
	    {-1, 1,  -1.0f, -1.0f, -3.1415f * 0.75f}, // 左下
	    {1,  1,  1.0f,  -1.0f, 3.1415f * 0.75f }  // 右下
	};

	int arrowCount = 0;

	for (int i = 0; i < 8; ++i) {
		const auto& dir = dirs[i];

		int nextX = x + dir.checkX;
		int nextZ = z + dir.checkZ;

		if (nextX >= 0 && nextZ >= 0) {
			auto type = mapChipField_->GetMapChipTypeByIndex(nextX, nextZ, 0);
			if (type == MapChipType::kBlock) {

				if (arrowCount >= 8)
					break;

				guideTransforms_[arrowCount].scale_ = {1.0f, 1.0f, 1.0f};
				guideTransforms_[arrowCount].rotation_ = {0.0f, dir.rotateY, 0.0f};

				guideTransforms_[arrowCount].translation_ = worldTransform_.translation_ + Vector3(dir.dispX * offsetDistance, 0.0f, dir.dispZ * offsetDistance);
				guideTransforms_[arrowCount].translation_.y = arrowHeight;

				WorldTransformUpdate(guideTransforms_[arrowCount]);

				activeGuides.push_back(&guideTransforms_[arrowCount]);
				arrowCount++;
			}
		}
	}

	return activeGuides;
}