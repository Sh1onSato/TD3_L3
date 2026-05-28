#pragma once
#include "KamataEngine.h"
#include "Fade.h"

using namespace KamataEngine;

class StageSelectScene {
public:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};

	~StageSelectScene();

	void Initialize();

	void Update();

	void Draw();

	bool IsFinished() const { return finished_; }

	/// @brief 選択中のステージ番号を取得
	int GetSelectedStage() const { return currentStage_; }

private:
	// ビュープロジェクション
	Camera camera_;
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	
	bool finished_ = false;

	Fade* fade_ = nullptr;

	Phase phase_ = Phase::kFadeIn;

	// ステージ選択用
	int currentStage_ = 0;           // 現在選択中のステージ (0, 1, 2...)
	static inline const int kMaxStage = 6;     // 最大ステージ数
	static inline const int kStagesPerRow = 6; // 1行あたりのステージ数（全ステージを1行表示）
	float targetX_ = 0.0f;           // プレイヤーの目標X座標
	float targetY_ = 0.0f;           // プレイヤーの目標Y座標

	WorldTransform stageTransforms_[kMaxStage]; // ステージのポイント
	Model* stageModel_ = nullptr;               // ステージのモデル
};
