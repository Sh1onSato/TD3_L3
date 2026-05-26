#pragma once
#include "KamataEngine.h"
#include "Math.h"

class MapChipField;

/// @brief 壊せる箱クラス
class Box {
public:
	/// @brief 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, MapChipField* mapChipField, uint32_t xIndex, uint32_t yIndex, uint32_t layer);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 破壊処理
	void OnCollision();

	// --- ゲッター ---
	bool IsAlive() const { return alive_; }
	uint32_t GetLayer() const { return layer_; }
	uint32_t GetXIndex() const { return xIndex_; }
	uint32_t GetYIndex() const { return yIndex_; }
	bool IsFalling() const { return isFalling_; }
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

	MapChipField* mapChipField_ = nullptr;
	uint32_t xIndex_ = 0;
	uint32_t yIndex_ = 0;
	uint32_t layer_ = 0;

	bool alive_ = true; // 壊れていないか
	bool isFalling_ = false; // 落下中か
	KamataEngine::Vector3 velocity_ = {0, 0, 0};
};