#include "StageSelectScene.h"
#include "Math.h"

StageSelectScene::~StageSelectScene() {
	delete model_;
	delete stageModel_;
	delete fade_;
}

void StageSelectScene::Initialize() {
	camera_.Initialize();
	// カメラを近づける
	camera_.translation_ = { 0.0f, 0.0f, -17.0f };
	camera_.rotation_ = { 0.0f, 0.0f, 0.0f };
	
	model_ = Model::CreateFromOBJ("player");
	stageModel_ = Model::CreateFromOBJ("cube");
	
	worldTransform_.Initialize();
	
	// ステージポイントの配置 (5x2のグリッド)
	float intervalX = 5.0f;
	float intervalY = 4.0f;
	float offsetX = (float)(5 - 1) * intervalX / 2.0f;
	float offsetY = (float)(2 - 1) * intervalY / 2.0f;

	for (int i = 0; i < kMaxStage; i++) {
		stageTransforms_[i].Initialize();
		// 行と列の計算
		int row = i / 5; // 0:上段, 1:下段
		int col = i % 5; // 0~4:左から右
		
		stageTransforms_[i].translation_ = { 
			(float)col * intervalX - offsetX, 
			offsetY - (float)row * intervalY, // 上段をプラス、下段をマイナスに
			0.0f 
		};
		stageTransforms_[i].scale_ = { 0.5f, 0.5f, 0.5f };
		WorldTransformUpdate(stageTransforms_[i]);
	}
	
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 0.5f);

	currentStage_ = 0;
	targetX_ = stageTransforms_[currentStage_].translation_.x;
	targetY_ = stageTransforms_[currentStage_].translation_.y;
}

void StageSelectScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		// 十字キー左右で列移動
		if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
			if (currentStage_ % 5 > 0) {
				currentStage_--;
			}
		}
		if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
			if (currentStage_ % 5 < 4) {
				currentStage_++;
			}
		}
		// 十字キー上下で行移動
		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			if (currentStage_ >= 5) {
				currentStage_ -= 5;
			}
		}
		if (Input::GetInstance()->TriggerKey(DIK_DOWN)) {
			if (currentStage_ < 5) {
				currentStage_ += 5;
			}
		}

		// スペースキーで決定
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 0.5f);
			phase_ = Phase::kFadeOut;
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	// ステージのスケールを更新（選択中を大きく、それ以外を小さく）
	for (int i = 0; i < kMaxStage; i++) {
		float targetScale = (i == currentStage_) ? 0.8f : 0.5f;
		// スムーズに変化させる
		stageTransforms_[i].scale_.x += (targetScale - stageTransforms_[i].scale_.x) * 0.2f;
		stageTransforms_[i].scale_.y += (targetScale - stageTransforms_[i].scale_.y) * 0.2f;
		stageTransforms_[i].scale_.z += (targetScale - stageTransforms_[i].scale_.z) * 0.2f;
		WorldTransformUpdate(stageTransforms_[i]);
	}

	// 目標座標の取得（ステージの右下あたりにオフセット）
	targetX_ = stageTransforms_[currentStage_].translation_.x + 1.2f;
	targetY_ = stageTransforms_[currentStage_].translation_.y - 1.2f;

	// なめらかな移動 (線形補間)
	worldTransform_.translation_.x += (targetX_ - worldTransform_.translation_.x) * 0.15f;
	worldTransform_.translation_.y += (targetY_ - worldTransform_.translation_.y) * 0.15f;

	// モデルを回転
	worldTransform_.rotation_.y += 0.03f;
	
	WorldTransformUpdate(worldTransform_);
	camera_.UpdateMatrix();
	camera_.TransferMatrix();
}

void StageSelectScene::Draw() {
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	Model::PreDraw(commandList);
	
	// 各ステージポイントの描画
	for (int i = 0; i < kMaxStage; i++) {
		stageModel_->Draw(stageTransforms_[i], camera_);
	}
	
	// 選択プレイヤーの描画
	model_->Draw(worldTransform_, camera_);
	
	Model::PostDraw();

	fade_->Draw();
}
