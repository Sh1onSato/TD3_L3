#include "GameScene.h"
#include "Math.h"
#include <string>

using namespace KamataEngine;

/**
 * @brief デストラクタ
 */
GameScene::~GameScene() { 
	delete model_;
	delete player_;
	delete player2_;
	delete debugCamera_;
	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
	delete fade_;
	delete interface_;

	for (auto& layer : worldTransformBlocks_) {
		for (auto& line : layer) {
			for (WorldTransform* block : line) {
				delete block;
			}
		}
	}
	worldTransformBlocks_.clear();
	
	delete deathParticles_;
	delete deathParticles2_;
	
	delete playerModel_;
	delete floorBlockModel_;
	delete ravageBlockModel_;
	delete skydomeModel_;
	delete deathParticleModel_;
	delete interfaceModel_;
	delete arrowModel_;

	for (Box* box : boxes_) {
		delete box;
	}
}

/**
 * @brief 初期化
 */
void GameScene::Initialize(int stageIndex) {
	stageIndex_ = stageIndex;

	// --- 1. システム・カメラの初期化 ---
	camera_.Initialize();
	// カメラを斜め上からの俯瞰視点に設定
	camera_.translation_ = { 0.0f, 15.0f, -10.0f }; // 高く、手前に
	camera_.rotation_ = { 0.8f, 0.0f, 0.0f };      // 下を向く
	
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);
	
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// --- 2. モデルデータのロード ---
	model_ = Model::Create();
	skydomeModel_ = Model::CreateFromOBJ("skydome", true);
	floorBlockModel_ = Model::CreateFromOBJ("floorBlocks");
	ravageBlockModel_ = Model::CreateFromOBJ("ravageBlocks");
	playerModel_ = Model::CreateFromOBJ("player");
	deathParticleModel_ = Model::CreateFromOBJ("deathParticle");
	interfaceModel_ = Model::CreateFromOBJ("player");
	arrowModel_ = Model::CreateFromOBJ("arrow");

	// --- 3. マップの生成 ---
	mapChipField_ = new MapChipField();
	mapChipField_->ResetMapChipData();

	std::string stageName = "STAGE" + std::to_string(stageIndex_ + 1);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/" + stageName + "/floorBlocks.csv", 0);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/" + stageName + "/ravageBlocks.csv", 1);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/" + stageName + "/upperBlocks.csv", 2);

	GenerateBlocks();

	// --- 4. プレイヤーの生成と初期化 ---
	player_ = new Player();
	player_->SetMapChipField(mapChipField_);
	// マップ上の初期位置（インデックス 0, 0）
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0, 0);
	player_->Initialize(playerModel_, &camera_, playerPosition);

	// ゆか（プレイヤー2）の生成と初期化 - 対角のマスに配置
	player2_ = new Player();
	player2_->SetPlayerIndex(1);
	player2_->SetMapChipField(mapChipField_);
	Vector3 player2Position = mapChipField_->GetMapChipPositionByIndex(9, 9);
	player2_->Initialize(playerModel_, &camera_, player2Position);

	// --- 5. 背景（スカイドーム）の初期化 ---
	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, &camera_);

	// --- 6. カメラコントローラーの初期化 ---
	cameraController_ = new CameraController(); 
	cameraController_->Initialize(&camera_);    
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	// --- 7. インターフェースの初期化 ---
	interface_ = new Interface();
	interface_->Initialize(interfaceModel_, &camera_);
	// カメラの移動可能範囲（XZ平面に合わせて調整）
	CameraController::Rect cameraArea = {0.0f, 100.0f, 0.0f, 20.0f};
	cameraController_->SetMovableArea(cameraArea);

	phase_ = Phase::kFadeIn;
}

/**
 * @brief フェーズの変更判定
 */
void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		// どちらかのプレイヤーが死んだら死亡演出へ
		if (player_->IsDead() && !deathParticles_) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticleModel_, &camera_, deathParticlesPosition);
		} else if (player2_->IsDead() && !deathParticles2_) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition2 = player2_->GetWorldPosition();
			deathParticles2_ = new DeathParticles;
			deathParticles2_->Initialize(deathParticleModel_, &camera_, deathParticlesPosition2);
		}
		// 両プレイヤーの移動回数が0のとき、Rキーでリセット
		else if (player_->GetRemainingMoves() <= 0 && player2_->GetRemainingMoves() <= 0 && Input::GetInstance()->TriggerKey(DIK_R)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		else {
			// すべての箱が壊れたかチェック (ravageBlocks)
			bool allBroken = true;
			int boxCount = 0;
			for (Box* box : boxes_) {
				boxCount++;
				if (box->IsAlive()) {
					allBroken = false;
					break;
				}
			}

			if (boxCount > 0 && allBroken && !player_->IsMoving() && !player2_->IsMoving()) {
				phase_ = Phase::kFadeOut;
				fade_->Start(Fade::Status::FadeOut, 1.0f);
			}
		}
		break;
	case Phase::kDeath:
		{
			bool p1Done = !deathParticles_ || deathParticles_->IsFinished();
			bool p2Done = !deathParticles2_ || deathParticles2_->IsFinished();
			if (p1Done && p2Done) {
				phase_ = Phase::kFadeOut;
				fade_->Start(Fade::Status::FadeOut, 1.0f);
			}
		}
		break;
	case Phase::kFadeOut:
		if (fade_->IsFinished()) {
			// クリア条件の再確認
			bool allBroken = true;
			int boxCount = 0;
			for (Box* box : boxes_) {
				boxCount++;
				if (box->IsAlive()) {
					allBroken = false;
					break;
				}
			}

			// クリアしていたら終了（タイトルへ）
			if (boxCount > 0 && allBroken) {
				finished_ = true;
			}
			// どちらかのプレイヤーが死亡、または両プレイヤーの手数が尽きた場合はリセット
			else if (player_->IsDead() || player2_->IsDead() ||
			         (player_->GetRemainingMoves() <= 0 && player2_->GetRemainingMoves() <= 0)) {
				Reset();
				phase_ = Phase::kFadeIn;
				fade_->Start(Fade::Status::FadeIn, 1.0f);
			} else {
				finished_ = true;
			}
		}
		break;
	}
}

/**
 * @brief ゲームをリセット（最初からやり直し）
 */
void GameScene::Reset() {
	// マップデータの再読み込み
	mapChipField_->ResetMapChipData();
	
	std::string stageName = "STAGE" + std::to_string(stageIndex_ + 1);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/" + stageName + "/floorBlocks.csv", 0);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/" + stageName + "/ravageBlocks.csv", 1);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/" + stageName + "/upperBlocks.csv", 2);

	// 既存のブロックWorldTransformをクリアして再生成
	for (auto& layer : worldTransformBlocks_) {
		for (auto& line : layer) {
			for (WorldTransform* block : line) {
				delete block;
			}
			line.clear();
		}
		layer.clear();
	}
	worldTransformBlocks_.clear();

	// 既存の箱を削除
	for (Box* box : boxes_) {
		delete box;
	}
	boxes_.clear();

	GenerateBlocks();

	// プレイヤーの位置と状態を初期化
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0, 0);
	player_->Initialize(playerModel_, &camera_, playerPosition);
	player_->SetRemainingMoves(10); // 回数をリセット

	// ゆか（プレイヤー2）の位置と状態を初期化
	Vector3 player2Position = mapChipField_->GetMapChipPositionByIndex(9, 9);
	player2_->Initialize(playerModel_, &camera_, player2Position);
	player2_->SetRemainingMoves(10);

	// カメラの初期化
	cameraController_->Reset();

	// 死亡エフェクトがあれば消す
	if (deathParticles_) {
		delete deathParticles_;
		deathParticles_ = nullptr;
	}
	if (deathParticles2_) {
		delete deathParticles2_;
		deathParticles2_ = nullptr;
	}
}

/**
 * @brief マップデータに基づいてブロックを生成
 */
void GameScene::GenerateBlocks() {
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// レイヤー 0:床 (WorldTransform)
	worldTransformBlocks_.resize(1); 
	worldTransformBlocks_[0].resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		worldTransformBlocks_[0][i].resize(numBlockHorizontal, nullptr);
	}

	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			// レイヤー 0: 床
			if (mapChipField_->GetMapChipTypeByIndex(j, i, 0) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransform->translation_.y = 0.0f;
				worldTransformBlocks_[0][i][j] = worldTransform;
			}

			// レイヤー 1 & 2: 破壊可能なブロック (Box)
			for (uint32_t layer = 1; layer < 3; ++layer) {
				if (mapChipField_->GetMapChipTypeByIndex(j, i, layer) == MapChipType::kBlock) {
					Box* newBox = new Box();
					Vector3 position = mapChipField_->GetMapChipPositionByIndex(j, i);
					position.y = static_cast<float>(layer); // 高さをレイヤーに合わせる
					newBox->Initialize(ravageBlockModel_, &camera_, position, mapChipField_, j, i, layer);
					boxes_.push_back(newBox);
				}
			}
		}
	}
}

/**
 * @brief 更新
 */
void GameScene::Update() { 
	ChangePhase();

	fade_->Update();
	skydome_->Update();
	cameraController_->Update();
	
	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
		}
		player_->Update();
		break;

	case Phase::kPlay:
		player_->Update();
		player2_->Update();
		CheckAllCollisions();

		for (Box* box : boxes_) {
			box->Update();
		}

		interface_->Update(player_->GetRemainingMoves());

		break;

	case Phase::kDeath:
		if (deathParticles_) {
			deathParticles_->Update();
		}
		if (deathParticles2_) {
			deathParticles2_->Update();
		}
		break;

	case Phase::kFadeOut:
		break;
	}

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	for (auto& layer : worldTransformBlocks_) {
		for (auto& line : layer) {
			for (auto& block : line) {
				if (block) WorldTransformUpdate(*block);
			}
		}
	}
}

/**
 * @brief 描画
 */
void GameScene::Draw() { 
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	skydome_->Draw(); 

	if (!player_->IsDead()) {
		player_->Draw(); 
	}
	if (!player2_->IsDead()) {
		player2_->Draw();
	}

	for (uint32_t layer = 0; layer < worldTransformBlocks_.size(); ++layer) {
		for (uint32_t i = 0; i < worldTransformBlocks_[layer].size(); ++i) {
			for (uint32_t j = 0; j < worldTransformBlocks_[layer][i].size(); ++j) {
				WorldTransform* block = worldTransformBlocks_[layer][i][j];
				if (block && mapChipField_->GetMapChipTypeByIndex(j, i, layer) != MapChipType::kBlank) {
					floorBlockModel_->Draw(*block, camera_);
				}
			}
		}
	}

	if (player_->IsDead() && deathParticles_) {
		deathParticles_->Draw();
	}
	if (player2_->IsDead() && deathParticles2_) {
		deathParticles2_->Draw();
	}

	for (Box* box : boxes_) {
		box->Draw();
	}

	interface_->Draw(player_->GetRemainingMoves());

	
	/*
		if (!player_->IsDead() && arrowModel_) {
		// 戻り値をポインタの vector に変更
		std::vector<KamataEngine::WorldTransform*> arrowTransforms = player_->GetGuideTransforms();

		if (!arrowTransforms.empty()) {
			for (const auto& transform : arrowTransforms) {
				// transform はポインタなので、*transform にして実体を渡して描画します
				arrowModel_->Draw(*transform, camera_);
			}
		}
	}
	*/

	Model::PostDraw();

	// フェードの描画
	fade_->Draw();

	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();
}

/**
 * @brief 当たり判定のチェック
 */
void GameScene::CheckAllCollisions() {
	// プレイヤー1（シオン）の当たり判定
	Vector3 playerPos = player_->GetWorldPosition();

	for (Box* box : boxes_) {
		if (!box->IsAlive()) continue;
		if (box->GetLayer() != 1) continue;

		Vector3 boxPos = mapChipField_->GetMapChipPositionByIndex(box->GetXIndex(), box->GetYIndex());

		float dx = std::abs(playerPos.x - boxPos.x);
		float dz = std::abs(playerPos.z - boxPos.z);

		if (dx < 0.4f && dz < 0.4f) {
			box->OnCollision();
		}
	}

	// プレイヤー2（ゆか）の当たり判定
	Vector3 player2Pos = player2_->GetWorldPosition();

	for (Box* box : boxes_) {
		if (!box->IsAlive()) continue;
		if (box->GetLayer() != 1) continue;

		Vector3 boxPos = mapChipField_->GetMapChipPositionByIndex(box->GetXIndex(), box->GetYIndex());

		float dx = std::abs(player2Pos.x - boxPos.x);
		float dz = std::abs(player2Pos.z - boxPos.z);

		if (dx < 0.4f && dz < 0.4f) {
			box->OnCollision();
		}
	}
}

