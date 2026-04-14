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

	/// @brief 破壊処理
	void OnCollision();

	// --- ゲッター ---
	bool IsAlive() const { return alive_; }
	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();


	void SetonGround(bool onGround) { onGround_ = onGround; }
	Vector3 GetPosition() const { return worldTransform_.translation_; }
	void SetPosition(const Vector3& pos) { worldTransform_.translation_ = pos; }

	// --- 統計用（EnemyのclearCountと同じ仕組み） ---
	inline static int breakCount = 0;

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 position_;
	KamataEngine::Vector3 size_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;  

	Vector3 velocity_ = {0.0f, 0.0f, 0.0f};

	bool alive_ = true; // 壊れていないか

	bool onGround_ = false;//一番下の段か
};