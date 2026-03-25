#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

// 前方宣言（循環参照を防ぐ）
class MapChipField;
class Enemy;

/**
 * @brief 自キャラ（プレイヤー）クラス
 */
class Player {
public:
	// --- 列挙型 ---
	
	/// @brief 左右の向き
	enum class LRDirection {
		kRight, // 右
		kLeft,  // 左
	};

	/// @brief 当たり判定用の4角のインデックス
	enum Corner { 
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上
		kNumCorner    // 角の数
	};

	// --- 公開メンバ関数 ---

	/// @brief 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const Vector3& position);
	
	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 当たり判定（AABB）を取得
	AABB GetAABB();

	/// @brief 衝突時のコールバック
	void OnCollision(const Enemy* enemy);

	// --- アクセッサ ---
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	const Vector3& GetVelocity() const { return velocity_; }
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }
	Vector3 GetWorldPosition();
	bool IsDead() const { return isDead_; }

private:
	// --- 内部処理用関数 ---

	/// @brief 移動入力の処理
	void InputMove();

	/// @brief マップとの当たり判定
	struct CollisionMapInfo {
		bool ceiling = false; // 天井にぶつかったか
		bool landing = false; // 着地したか
		bool hitWall = false; // 壁にぶつかったか
		Vector3 move;         // 補正後の移動量
	};
	void CheckMapCollision(CollisionMapInfo& info);
	
	// 各方向の当たり判定
	void CheckMapCollisionUp(CollisionMapInfo& info);
	void CheckMapCollisionDown(CollisionMapInfo& info);
	void CheckMapCollisionRight(CollisionMapInfo& info);
	void CheckMapCollisionLeft(CollisionMapInfo& info);

	/// @brief 指定した角の座標を計算
	Vector3 CornerPosition(const Vector3& center, Corner corner);

	/// @brief 接地状態の更新
	void UpdateOnGround(const CollisionMapInfo& info);

	/// @brief 壁接触状態の更新
	void UpdateOnWall(const CollisionMapInfo& info);

	
private:
	// --- 基本コンポーネント ---
	KamataEngine::WorldTransform worldTransform_; // ワールド座標データ
	KamataEngine::Camera* camera_ = nullptr;     // カメラへの参照
	KamataEngine::Model* model_ = nullptr;       // 使用するモデル
	uint32_t textureHandle_ = 0u;                // テクスチャ

	// --- 移動・ステータス ---
	Vector3 velocity_ = {0, 0, 0};   // 現在の移動速度
	LRDirection lrDirection_ = LRDirection::kRight; // 向いている方向
	bool onGround_ = true;           // 地面に足がついているか
	bool isDead_ = false;            // 死亡フラグ
	MapChipField* mapChipField_ = nullptr; // マップデータへの参照

	// --- 演出・タイマー関連 ---
	//float turnFirstRotationY_ = 0.0f; // 旋回開始時の角度
	//float turnTimer_ = 0.0f;          // 旋回のアニメーション用タイマー

	// --- マス移動アニメーション ---
	bool isMoving_ = false;           // マス移動中か
	Vector3 moveStartPosition_ = {};  // 移動開始位置
	Vector3 moveTargetPosition_ = {}; // 移動目標位置（マスの中心）
	float moveTimer_ = 0.0f;          // 移動アニメーションタイマー


	// --- 定数（調整パラメータ） ---
	//static inline const float kAcceleration = 0.01f;      // 加速
	//static inline const float kAttenuation = 0.05f;       // 減速
	//static inline const float kLimitRunSpeed = 0.3f;      // 最大走行速度

	static inline const float kMoveTime = 0.15f;          // 1マス移動にかかる時間（秒）
	static inline const float kTimeTurn = 0.3f;           // 旋回にかかる時間
	
	//static inline const float kJumpAcceleration = 20.0f;  // ジャンプ力（初速）
	//static inline const float kGravityAcceleration = 0.98f; // 重力
	//static inline const float kLimitFallSpeed = 0.5f;     // 最大落下速度
	static inline const float kWidth = 0.8f;              // キャラクターの横幅（判定用）
	static inline const float kHeight = 0.8f;             // キャラクターの高さ（判定用）
	static inline const float kBlank = 0.04f;             // 判定の遊び（少し小さくする）
	static inline const float kGroundSearchHeight = 0.06f; // 接地判定の深さ
	static inline const float kAttenuationLanding = 0.0f;  // 着地時の減速
	static inline const float kAttenuationWall = 0.2f;     // 壁接触時の減速
};
