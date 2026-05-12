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
	isFlying_ = false;
	flyVelocity_ = {};
	flyRotationVelocity_ = {};

}

void Box::Update() {
	if (!alive_ && !isFlying_) {
		return;
	}
	// 飛ばし演出中：重力を加えて位置と回転を更新
	if (isFlying_) {
		flyVelocity_.y -= kGravity;
		worldTransform_.translation_ += flyVelocity_;
		worldTransform_.rotation_ += flyRotationVelocity_;
		// 一定の高さより下に落ちたら演出終了
		if (worldTransform_.translation_.y < flyDisappearY_) {
			isFlying_ = false;
		}
	}

	// 行列の更新 (Enemy.cpp と同じ計算式)
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// 定数バッファに転送
	worldTransform_.TransferMatrix();
}

void Box::Draw() {
	if (alive_ || isFlying_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

void Box::OnCollision(const KamataEngine::Vector3& knockDirection) {
	if (alive_) {
		alive_ = false;
		isFlying_ = true;
		breakCount++; // 壊した数を加算

			// ノックバック方向に水平速度＋上方向への初速を設定
		flyVelocity_ = {
		    knockDirection.x * kKnockSpeed,
		    kKnockUpSpeed,
		    knockDirection.z * kKnockSpeed,
		};
		// 吹っ飛びながら回転（だるま落とし風のタンブリング）
		flyRotationVelocity_ = kFlyRotationVelocity;
		// 消える高さの目安を現在Y座標から設定
		flyDisappearY_ = worldTransform_.translation_.y - 3.0f;
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