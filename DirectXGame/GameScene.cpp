#include "GameScene.h"
#include "Math.h"

using namespace KamataEngine;

/**
 * @brief デストラクタ
 * 生成した動的メモリをすべて解放する。
 */
GameScene::~GameScene() { 
	delete model_;
	delete player_;
	delete debugCamera_;
	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
	delete fade_;

	// ブロックのメモリを解放
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	// 敵のメモリを解放
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	
	delete deathParticles_;
	
	// モデルデータの解放
	delete playerModel_;
	delete blockModel_;
	delete skydomeModel_;
	delete enemyModel_;
	delete deathParticleModel_;
}

/**
 * @brief 初期化
 */
void GameScene::Initialize() {
	// --- 1. システム・カメラの初期化 ---
	camera_.Initialize();
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
	
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f); // フェードインから開始

	// --- 2. モデルデータのロード ---
	model_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("skydome", true);
	blockModel_ = Model::CreateFromOBJ("block");
	playerModel_ = Model::CreateFromOBJ("player");
	enemyModel_ = Model::CreateFromOBJ("enemy");
	deathParticleModel_ = Model::CreateFromOBJ("deathParticle");

	// --- 3. マップの生成 ---
	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// --- 4. プレイヤーの生成と初期化 ---
	player_ = new Player();
	player_->SetMapChipField(mapChipField_);
	// マップ上の初期位置（2, 18）を取得して配置
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);
	player_->Initialize(playerModel_, &camera_, playerPosition);

	// --- 5. 背景（スカイドーム）の初期化 ---
	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, &camera_);

	// --- 6. カメラコントローラーの初期化 ---
	cameraController_ = new CameraController(); 
	cameraController_->Initialize(&camera_);    
	cameraController_->SetTarget(player_);
	cameraController_->Reset();
	// カメラの移動可能範囲を設定
	CameraController::Rect cameraArea = {12.0f, 100.0f - 12.0f, 6.0f, 6.0f};
	cameraController_->SetMovableArea(cameraArea);

	// --- 7. 敵の生成 ---
	for (int32_t i = 0; i < 2; ++i) {
		Enemy* newEnemy = new Enemy();
		// 初期位置をずらして配置
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(14 + i * 2, 18);
		newEnemy->Initialize(enemyModel_, &camera_, enemyPosition);
		enemies_.push_back(newEnemy);
	}

	// 最初の状態をプレイ中に設定
	phase_ = Phase::kPlay;
}

/**
 * @brief フェーズの変更判定
 */
void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		// プレイヤーが死んだら死亡演出へ
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;

			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticleModel_, &camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		// 死亡演出が終わったらフェードアウトへ
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;
	case Phase::kFadeOut:
		// フェードアウトが終わったらシーン終了
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}
}

/**
 * @brief マップデータに基づいてブロックを生成
 */
void GameScene::GenerateBlocks() {
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal, nullptr);
	}

	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			// ブロックタイプの場合のみ実体を作る
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = worldTransform;
			}
		}
	}
}

/**
 * @brief 更新
 */
void GameScene::Update() { 
	// フェーズの遷移チェック
	ChangePhase();

	// --- 共通の更新処理 ---
	skydome_->Update();
	cameraController_->Update();
	
	// --- フェーズごとの個別更新 ---
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
		}
		player_->Update();
		break;

	case Phase::kPlay:
		player_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		CheckAllCollisions(); // 当たり判定
		break;

	case Phase::kDeath:
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		if (deathParticles_) {
			deathParticles_->Update();
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}
		break;
	}

	// --- デバッグカメラの切り替え ---
#ifdef _DEBUG
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}
#endif

	// --- カメラの最終行列計算 ---
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	// --- ブロックのワールド行列更新 ---
	for (auto& line : worldTransformBlocks_) {
		for (auto& block : line) {
			if (block) WorldTransformUpdate(*block);
		}
	}
}

/**
 * @brief 描画
 */
void GameScene::Draw() { 
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// --- 3D描画 ---
	Model::PreDraw(dxCommon->GetCommandList());

	skydome_->Draw(); // 背景

	if (!player_->IsDead()) {
		player_->Draw(); // プレイヤー
	}

	// ブロック描画
	for (auto& line : worldTransformBlocks_) {
		for (auto& block : line) {
			if (block) blockModel_->Draw(*block, camera_);
		}
	}

	// 敵描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	// 死亡演出描画
	if (player_->IsDead() && deathParticles_) {
		deathParticles_->Draw();
	}

	Model::PostDraw();

	// --- 2D描画 ---
	Sprite::PreDraw(dxCommon->GetCommandList());
	
	// フェードの描画（必要に応じて有効化）
	// if (fade_) fade_->Draw();

	Sprite::PostDraw();
}

/**
 * @brief 当たり判定のチェック
 */
void GameScene::CheckAllCollisions() {
	AABB aabbPlayer = player_->GetAABB();

	// プレイヤーと敵の判定
	for (Enemy* enemy : enemies_) {
		AABB aabbEnemy = enemy->GetAABB();

		// 交差していたら衝突処理
		if (IsCollision(aabbPlayer, aabbEnemy)) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
}
