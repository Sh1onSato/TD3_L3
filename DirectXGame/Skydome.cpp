#include "Skydome.h"

void Skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	// 引数の内容をメンバ変数に記録
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
}

void Skydome::Update() { 
	worldTransform_.TransferMatrix(); 
}

void Skydome::Draw() { 
	model_->Draw(worldTransform_, *camera_);
}
