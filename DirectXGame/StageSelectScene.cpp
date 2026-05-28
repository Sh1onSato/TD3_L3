#include "StageSelectScene.h"
#include "Math.h"
#include <string>

StageSelectScene::~StageSelectScene() {
	delete model_;
	delete stageModel_;
	delete fade_;
}

void StageSelectScene::Initialize() {
	camera_.Initialize();
	// カメラをさらに近づける
	camera_.translation_ = { 0.0f, 0.0f, -17.0f };
	camera_.rotation_ = { 0.0f, 0.0f, 0.0f };
	
	model_ = Model::CreateFromOBJ("player");
	stageModel_ = Model::CreateFromOBJ("cube");
	
	worldTransform_.Initialize();
	
	// ステージポイントの配置 (6つを1行に横並び)
	float intervalX = 3.0f;
	float offsetX = (float)(kStagesPerRow - 1) * intervalX / 2.0f;

	for (int i = 0; i < kMaxStage; i++) {
		stageTransforms_[i].Initialize();
		int col = i % kStagesPerRow;
		
		stageTransforms_[i].translation_ = { 
			(float)col * intervalX - offsetX, 
			0.0f,
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
		// 十字キー左右でステージ移動
		if (Input::GetInstance()->TriggerKey(DIK_LEFT)) {
			if (currentStage_ > 0) {
				currentStage_--;
			}
		}
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

	// 目標座標の取得
	targetX_ = stageTransforms_[currentStage_].translation_.x;
	targetY_ = stageTransforms_[currentStage_].translation_.y;

	// なめらかな移動 (線形補間)
	worldTransform_.translation_.x += (targetX_ - worldTransform_.translation_.x) * 0.15f;
	worldTransform_.translation_.y += (targetY_ - worldTransform_.translation_.y) * 0.15f;

	// モデルを回転
	worldTransform_.rotation_.y += 0.03f;
	
	WorldTransformUpdate(worldTransform_);
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

	// 各ステージの番号をテキストで表示
	Sprite::PreDraw(commandList);
	for (int i = 0; i < kMaxStage; i++) {
		// ステージポイントの 3D 座標 → スクリーン座標に変換
		Vector3 pos3D = stageTransforms_[i].translation_;
		pos3D.y += 1.2f; // テキストをポイントの上にずらす
		Vector3 viewPos = Transform(pos3D, camera_.matView);
		Vector3 ndcPos = Transform(viewPos, camera_.matProjection);
		float screenX = (ndcPos.x + 1.0f) * 0.5f * (float)WinApp::kWindowWidth;
		float screenY = (1.0f - ndcPos.y) * 0.5f * (float)WinApp::kWindowHeight;

		static const float kTextOffsetX = -30.0f; // テキストの横方向センタリング調整
		std::string label = "Stage " + std::to_string(i + 1);
		DebugText::GetInstance()->Print(label, screenX + kTextOffsetX, screenY, 2.0f);
	}
	DebugText::GetInstance()->DrawAll();
	Sprite::PostDraw();

	fade_->Draw();
}
