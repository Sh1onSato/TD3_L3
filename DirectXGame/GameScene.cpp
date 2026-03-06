#include "GameScene.h"
#include"Math.h"

using namespace KamataEngine;
GameScene::~GameScene() { 
	delete model_;
	delete player_;
	delete debugCamera_;
	delete skydome_;
	delete mapChipField_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	delete deathParticles_;
	delete deathParticle_model_;
}

void GameScene::Initialize() {
	// 3Dモデルデータの生成
	model_ = Model::Create();
	// カメラの初期化
	camera_.Initialize();

	//スカイドームの生成
	skydome_ = new Skydome();
	// 3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	// スカイドームの初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// ブロックモデル
	block_model_ = Model::CreateFromOBJ("block");

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// 自キャラの生成
	player_ = new Player();
	// 自キャラの初期化
	player_model_ = Model::CreateFromOBJ("player");
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);

	player_->SetMapChipField(mapChipField_);

	player_->Initialize(player_model_, &camera_, playerPosition);


	CController_ = new CameraController(); 
	CController_->Initialize(&camera_);    
	CController_->SetTarget(player_);
	CController_->Reset();

	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	CController_->SetMovableArea(cameraArea);

	enemy_model_ = Model::CreateFromOBJ("enemy");

	for (int32_t i = 0; i < 2; ++i) {
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(14 + i * 2, 18);

		newEnemy->Initialize(enemy_model_, &camera_, enemyPosition);

		enemies_.push_back(newEnemy);
	}

	deathParticle_model_ = Model::CreateFromOBJ("deathParticle");

	phase_ = Phase::kPlay;

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// デバックカメラの生成
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		
		if (player_->IsDead()) {
			// 死亡演出
			phase_ = Phase::kDeath;

			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticle_model_, &camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		break;
	}
}

void GameScene::GenerateBlocks() {

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {

		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::Update() { 
	ChangePhase();

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		skydome_->Update();
		CController_->Update();

		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		// UpdateCamera();
#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			// フラグをトグル
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}
		break;
	case Phase::kPlay:
		skydome_->Update();
		CController_->Update();
		// 自キャラの更新
		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

#ifdef _DEBUG
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			// フラグをトグル
			isDebugCameraActive_ = !isDebugCameraActive_;
		}
#endif

		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}
;
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		CheckAllCollisions();
		break;
	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
		}

		skydome_->Update();
		CController_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		if (deathParticles_) {
			deathParticles_->Update();
		}

		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		skydome_->Update();
		CController_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		break;
	}
}

void GameScene::Draw() { 
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// スカイドームの描画
	skydome_->Draw();

	if (!player_->IsDead())
	player_->Draw();
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			block_model_->Draw(*worldTransformBlock, camera_);
		}
	}
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}
	if (player_->IsDead())
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	Model::PostDraw();
	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());
	// スプライト描画後処理
	Sprite::PostDraw();

	//fade_->Draw();
}

void GameScene::CheckAllCollisions() {

	// 判定対象1と2の座標
	AABB aabb1, aabb2;

#pragma region 自キャラと敵キャラの当たり判定
	{
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾全ての当たり判定
		for (Enemy* enemy : enemies_) {
			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_->OnCollision(enemy);
				// 敵弾の衝突時コールバックを呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion
}
