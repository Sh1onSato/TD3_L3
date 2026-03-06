#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

class MapChipField;
class Enemy;

///<sumary>
/// 自キャラ
/// </summary>
class Player {
	public :
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 角の位置
	enum Corner { 
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,
		kNumCorner 
	};

	float EaseInOut(float t, float start, float end);
	   
	//初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const Vector3& position);
	
	// 更新
	void Update();

	// 描画
	void Draw();

	const WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	Vector3 GetWorldPosition();

	AABB GetAABB();

	void OnCollision(const Enemy* enemy);

	bool IsDead() const { return isDead_; }
	private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_; 
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;
	// 移動速度
	Vector3 velocity_ = {};

	static inline const float kAcceleration = 0.01f;

	static inline const float kAttenuation = 0.05f;

	static inline const float kLimitRunSpeed = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;

	float turnFirstRotationY_ = 0.0f;

	float turnTimer_ = 0.0f;

	static inline const float kTimeTurn = 0.3f;

	bool onGround_ = true;

	static inline const float kJumpAcceleration = 20.0f;
	static inline const float kGravityAcceleration = 0.98f;
	static inline const float kLimitFallSpeed = 0.5f;

	MapChipField* mapChipField_ = nullptr;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	static inline const float kBlank = 0.04f;

	void InputMove();

	struct CollisionMapInfo {
		bool ceiling = false;
		bool landing = false;
		bool hitWall = false;
		Vector3 move;
	};

	void CheckMapCollision(CollisionMapInfo& info);

	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	Vector3 CornerPosition(const Vector3& center, Corner corner);

	void UpdateOnGround(const CollisionMapInfo& info);

	void UpdateOnWall(const CollisionMapInfo& info);

	static inline const float kAttenuationLanding = 0.0f;

	static inline const float kGroundSearchHeight = 0.06f;

	static inline const float kAttenuationWall = 0.2f;

	bool isDead_ = false;
};
