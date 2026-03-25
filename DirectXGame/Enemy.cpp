#include <cassert>
#include <numbers>
#include "Enemy.h"
#include "Math.h"

/**
 * @brief 初期化
 */
void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);

	model_ = model;
	camera_ = camera;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 初期方向を左に向ける（270度）
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	// 左に向かって歩き出す速度を設定
	velocity_ = {-kWalkSpeed, 0, 0};

	walkTimer = 0.0f;
}

/**
 * @brief 更新
 */
void Enemy::Update() {
	// 1. 移動：現在の速度を座標に加算
	worldTransform_.translation_ += velocity_;

	// 2. アニメーション：サイン波を使って体を左右に傾ける
	walkTimer += 1.0f / 60.0f;
	worldTransform_.rotation_.x = std::sin(std::numbers::pi_v<float> * 2.0f * walkTimer / kWalkMotionTime);

	// 3. 行列を更新
	WorldTransformUpdate(worldTransform_);
}

/**
 * @brief 描画
 */
void Enemy::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

/**
 * @brief 当たり判定用の範囲を取得
 */
AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb;

	// 中心の座標から、幅と高さの半分を引いたり足したりして範囲を決める
	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};
	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

/**
 * @brief ワールド座標の取得
 */
Vector3 Enemy::GetWorldPosition() {
	// ワールド行列から座標（m[3][0]~m[3][2]）を取り出す
	return {
		worldTransform_.matWorld_.m[3][0],
		worldTransform_.matWorld_.m[3][1],
		worldTransform_.matWorld_.m[3][2]
	};
}

/**
 * @brief プレイヤーと衝突した時の処理
 */
void Enemy::OnCollision(const Player* player) {
	(void)player;
	// 必要に応じて敵が倒れる処理などを書く
}
