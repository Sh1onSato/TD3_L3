#pragma once
#include "KamataEngine.h"
#include "Math.h"

class Player;

/// @brief 壊せる箱クラス
class Box {
public:

	struct Rect {
		float left;   // 左端
		float right;  // 右端
		float bottom; // 下端
		float top;    // 上端
	};

	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	/// @brief 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 破壊処理
	void OnCollision();

	//ゲッター
	bool IsAlive() const { return alive_; }
	KamataEngine::Vector3 GetWorldPosition();
	AABB GetAABB();

	// 統計用
	inline static int breakCount = 0;

	Rect GetRect();

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 position_;
	KamataEngine::Vector3 size_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;  

	bool alive_ = true; // 壊れていないか
};