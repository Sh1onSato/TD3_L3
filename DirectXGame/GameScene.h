#pragma once
#include "Player.h"
#include <vector>
#include <list>
#include "Skydome.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Fade.h"
#include "Box.h"

/**
 * @brief ゲーム本編のメインシーン
 */
class GameScene {
public:
	// --- 公開メンバ関数 ---
	
	~GameScene();

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief マップ上のブロックを生成
	void GenerateBlocks();

	/// @brief すべての当たり判定をチェック
	void CheckAllCollisions();

	/// @brief シーン終了フラグを取得
	bool IsFinished() const { return finished_; }

private:
	// --- 内部フェーズ管理 ---
	enum class Phase {
		kFadeIn,  // フェードイン
		kPlay,    // ゲームプレイ中
		kDeath,   // 死亡演出中
		kFadeOut, // フェードアウト
	};
	Phase phase_; // 現在の進行状況

	/// @brief フェーズの変更
	void ChangePhase();

	// --- 3Dモデルデータ ---
	KamataEngine::Model* model_ = nullptr;              // 汎用モデル
	KamataEngine::Model* playerModel_ = nullptr;        // プレイヤーのモデル
	KamataEngine::Model* blockModel_ = nullptr;         // ブロックのモデル
	KamataEngine::Model* skydomeModel_ = nullptr;       // スカイドームのモデル
	KamataEngine::Model* deathParticleModel_ = nullptr; // 死亡エフェクトのモデル

	// --- ゲームオブジェクト ---
	Player* player_ = nullptr;          // 自キャラ
	Skydome* skydome_ = nullptr;        // 背景の空
	MapChipField* mapChipField_ = nullptr; // マップデータ
	DeathParticles* deathParticles_ = nullptr; // 死亡エフェクト管理

	// --- カメラ・描画関連 ---
	KamataEngine::Camera camera_;               // メインカメラ
	CameraController* cameraController_ = nullptr; // カメラ制御
	KamataEngine::DebugCamera* debugCamera_ = nullptr; // デバッグ用カメラ
	bool isDebugCameraActive_ = false;          // デバッグカメラが有効か
	uint32_t textureHandle_ = 0;               // テクスチャハンドル

	// --- システム関連 ---
	Fade* fade_ = nullptr;      // 画面フェード演出
	bool finished_ = false;    // シーン終了フラグ
	
	// ブロックの座標情報などのリスト
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	//壊す箱
	std::vector<Box*> boxes_;
};
