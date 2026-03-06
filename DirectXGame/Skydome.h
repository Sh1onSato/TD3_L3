#pragma once
#include"KamataEngine.h"
class Skydome {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	void Update();

	void Draw();

	private:

	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;
	// カメラ
	KamataEngine::Camera* camera_;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;
};
