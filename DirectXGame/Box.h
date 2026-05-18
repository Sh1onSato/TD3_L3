#pragma once
#include "KamataEngine.h"
#include "Math.h"

class Player;

/// @brief 壊せる箱クラス
class Box {
public:
	/// @brief 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	// 破壊処理（ノックバック方向を受け取り飛ばす）
	void OnCollision(const KamataEngine::Vector3& knockDirection);

	// --- ゲッター ---
	bool IsAlive() const { return alive_; }
	bool IsFlying() const { return isFlying_; }
	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();

	// --- 統計用（EnemyのclearCountと同じ仕組み） ---
	inline static int breakCount = 0;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 position_;
	KamataEngine::Vector3 size_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	bool alive_ = true; // 壊れていないか

	// --- 飛ばし演出 ---
	bool isFlying_ = false;                          // 飛んでいる最中か
	KamataEngine::Vector3 flyVelocity_ = {};         // 飛ばされる速度（重力含む）
	KamataEngine::Vector3 flyRotationVelocity_ = {}; // 飛ばされながら回転する速度
	float flyDisappearY_ = -3.0f;                    // この高さを下回ったら非表示
	// --- 飛ばし演出の調整パラメータ ---
	static inline const float kKnockSpeed = 0.25f;                                          // 水平方向の吹っ飛び速度
	static inline const float kKnockUpSpeed = 0.18f;                                        // 初期上方向の速度
	static inline const float kGravity = 0.018f;                                            // 1フレームあたりの重力加速度
	static inline const KamataEngine::Vector3 kFlyRotationVelocity = {0.12f, 0.08f, 0.10f}; // タンブリング回転速度
};