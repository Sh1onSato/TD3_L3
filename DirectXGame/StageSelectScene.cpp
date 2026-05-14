#include "StageSelectScene.h"
#include "Math.h"

StageSelectScene::~StageSelectScene() {
	delete model_;
	delete fade_;
}

void StageSelectScene::Initialize() {
	camera_.Initialize();
	// カメラを少し引いた位置に設定
	camera_.translation_ = { 0.0f, 2.0f, -15.0f };
	camera_.rotation_ = { 0.1f, 0.0f, 0.0f };
	
	model_ = Model::CreateFromOBJ("player");
	worldTransform_.Initialize();
	
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 0.5f);

	currentStage_ = 0;
	targetX_ = 0.0f;
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
		// 十字キー左でステージ戻る
		if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
			if (currentStage_ > 0) {
				currentStage_--;
			}
		}
		// 十字キー右でステージ進む
		if (Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
			if (currentStage_ < kMaxStage - 1) {
				currentStage_++;
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

	// 目標X座標の計算 (ステージごとに5.0ずつ離す例)
	targetX_ = (float)currentStage_ * 5.0f - ((float)(kMaxStage - 1) * 5.0f / 2.0f);

	// なめらかな移動 (線形補間)
	worldTransform_.translation_.x += (targetX_ - worldTransform_.translation_.x) * 0.1f;

	// モデルを回転
	worldTransform_.rotation_.y += 0.03f;
	
	WorldTransformUpdate(worldTransform_);
	camera_.TransferMatrix();
}

void StageSelectScene::Draw() {
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	Model::PreDraw(commandList);
	model_->Draw(worldTransform_, camera_);
	Model::PostDraw();

	fade_->Draw();
}
