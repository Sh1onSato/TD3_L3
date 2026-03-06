#pragma once
#include "KamataEngine.h"
#include "Fade.h"

using namespace KamataEngine;

class TitleScene {
public:
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};
	~TitleScene();

	void Initialize();

	void Update();

	void Draw();

	bool IsFinished() const { return finished_; }

private:
	static inline const float kTimeTitleMove = 2.0f;

	// ビュープロジェクション
	Camera camera_;
	WorldTransform worldTransformTitle_;
	WorldTransform worldTransformPlayer_;

	Model* modelPlayer_ = nullptr;
	Model* modelTitle_ = nullptr;

	float counter_ = 0.0f;

	bool finished_ = false;

	Fade* fade_ = nullptr;

	Phase phase_ = Phase::kFadeIn;
};
