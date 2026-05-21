#include "Box.h"
#include "Math.h"
#include "MapChipField.h"

using namespace KamataEngine;

void Box::Initialize(Model* model, KamataEngine::Camera* camera, const Vector3& position, MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex, uint32_t layer) {
	assert(model);
	model_ = model;
	camera_ = camera;
	position_ = position;
	mapChipField_ = mapChipField;
	xIndex_ = xIndex;
	yIndex_ = yIndex;
	layer_ = layer;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	alive_ = true;
	isFalling_ = false;
	velocity_ = {0, 0, 0};
}

void Box::Update() {
	if (!alive_) {
		return;
	}

	// 落下チェック
	if (!isFalling_ && layer_ > 0) {
		MapChipType typeBelow = mapChipField_->GetMapChipTypeByIndex(xIndex_, yIndex_, layer_ - 1);
		if (typeBelow == MapChipType::kBlank) {
			isFalling_ = true;
			// 現在の場所を空白にする
			mapChipField_->SetMapChipTypeByIndex(xIndex_, yIndex_, MapChipType::kBlank, layer_);
		}
	}

	if (isFalling_) {
		const float kGravity = -0.01f;
		velocity_.y += kGravity;
		worldTransform_.translation_.y += velocity_.y;

		// 下のレイヤーの目標高さ
		float targetY = static_cast<float>(layer_ - 1);
		if (worldTransform_.translation_.y <= targetY) {
			worldTransform_.translation_.y = targetY;
			isFalling_ = false;
			velocity_.y = 0;
			layer_--; // レイヤーを下げる
			// 新しい場所をブロックにする
			mapChipField_->SetMapChipTypeByIndex(xIndex_, yIndex_, MapChipType::kBlock, layer_);
		}
	}

	// 行列の更新
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
	if (alive_ && !isFalling_) {
		alive_ = false;
		breakCount++; // 壊した数を加算
		// マップチップのデータを更新（空白にする）
		mapChipField_->SetMapChipTypeByIndex(xIndex_, yIndex_, MapChipType::kBlank, layer_);
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