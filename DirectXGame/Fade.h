#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Fade {
public:
	enum class Status {
		None,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト中
	};

	void Initialize();
	void Update();
	void Draw();

	void Start(Status status, float duration);

	void Stop();

	bool IsFinished() const;

private:
	Sprite* sprite_ = nullptr;

	Status status_ = Status::None;

	float duration_ = 0.0f;

	 float counter_ = 0.0f;
};
