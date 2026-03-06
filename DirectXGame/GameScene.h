#pragma once
#include"Player.h"
#include<vector>
#include"Skydome.h"
#include"MapChipField.h"
#include "CameraController.h"
#include "Enemy.h"
#include "DeathParticles.h"
#include "Fade.h"

// ゲームシーン
class GameScene {
public:
	~GameScene();
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	void GenerateBlocks();

	void CheckAllCollisions();

	bool IsFinished() const { return finished_; }

private:
	enum class Phase {
		kFadeIn, // フェードイン
		kPlay,  // ゲームプレイ
		kDeath, // デス演出
		kFadeOut, // フェードアウト
	};
	Phase phase_;

	void ChangePhase();

	//テクスチャハンドル
	uint32_t textureHandle_ = 0;
	//3Dモデルデータ
	KamataEngine::Model* model_ = nullptr;

	//カメラ
	KamataEngine::Camera camera_;
	// 自キャラ
	Player* player_ = nullptr;
	// プレイヤーモデル
	KamataEngine::Model* player_model_ = nullptr;

	// ブロックモデル
	KamataEngine::Model* block_model_ = nullptr;
	std::vector<std::vector<WorldTransform*>> worldTransformBlocks_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;
	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;


	// スカイドーム
	Skydome* skydome_ = nullptr;

	// 3Dモデル
	KamataEngine:: Model* modelSkydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	CameraController* CController_ = nullptr;

	Enemy* enemy_ = nullptr;

	KamataEngine::Model* enemy_model_ = nullptr;

	std::list<Enemy*> enemies_;

	DeathParticles* deathParticles_ = nullptr;

	Model* deathParticle_model_ = nullptr;

	bool finished_ = false;

	Fade* fade_ = nullptr;
};
