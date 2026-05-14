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
	static inline const int kMaxStage = 3; // 最大ステージ数
	float targetX_ = 0.0f;           // プレイヤーの目標X座標
};
