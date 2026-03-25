#pragma once

#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

// 前方宣言
class Player;

/**
 * @brief 敵キャラクラス
 */
class Enemy {
public:
	/// @brief 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 当たり判定（AABB）を取得
	AABB GetAABB();

	/// @brief ワールド座標を取得
	Vector3 GetWorldPosition();

	/// @brief 衝突時のコールバック
	void OnCollision(const Player* player);

private:
	// --- コンポーネント ---
	WorldTransform worldTransform_; // ワールド座標データ
	Model* model_ = nullptr;       // 敵のモデル
	Camera* camera_ = nullptr;     // カメラへの参照

	// --- 移動・アニメーション関連 ---
	Vector3 velocity_ = {};        // 移動速度
	float walkTimer = 0.0f;        // 歩行アニメーション用タイマー

	// --- 定数（調整パラメータ） ---
	static inline const float kWalkSpeed = 0.02f;         // 歩くスピード
	static inline const float kWalkMotionAngleStart = 0.0f; // 歩行時の傾き開始
	static inline const float kWalkMotionAngleEnd = 30.0f;   // 歩行時の傾き終了
	static inline const float kWalkMotionTime = 1.0f;       // アニメーションの周期
	static inline const float kWidth = 0.8f;               // 当たり判定の幅
	static inline const float kHeight = 0.8f;              // 当たり判定の高さ
};
