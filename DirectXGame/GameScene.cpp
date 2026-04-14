#include "GameScene.h"
#include "Math.h"

using namespace KamataEngine;

/**
 * @brief デストラクタ
 */
GameScene::~GameScene() { 
	delete model_;
	delete player_;
	delete debugCamera_;
	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
	delete fade_;

	for (auto& layer : worldTransformBlocks_) {
		for (auto& line : layer) {
			for (WorldTransform* block : line) {
				delete block;
			}
		}
	}
	worldTransformBlocks_.clear();
	
	delete deathParticles_;
	
	delete playerModel_;
	delete blockModel_;
	delete skydomeModel_;
	delete deathParticleModel_;

	for (Box* box : boxes_) {
		delete box;
	}
}

/**
 * @brief 初期化
 */
void GameScene::Initialize() {
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
	blockModel_ = Model::CreateFromOBJ("block");
	playerModel_ = Model::CreateFromOBJ("player");
	deathParticleModel_ = Model::CreateFromOBJ("deathParticle");

	// --- 3. マップの生成 ---
	mapChipField_ = new MapChipField();
	mapChipField_->ResetMapChipData();
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/floorBlocks.csv", 0);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/ravageBlocks.csv", 1);
	GenerateBlocks();

	// --- 4. プレイヤーの生成と初期化 ---
	player_ = new Player();
	player_->SetMapChipField(mapChipField_);
	// マップ上の初期位置（インデックス 0, 0）
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0, 0);
	player_->Initialize(playerModel_, &camera_, playerPosition);

	// --- 5. 背景（スカイドーム）の初期化 ---
	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, &camera_);

	// --- 6. カメラコントローラーの初期化 ---
	cameraController_ = new CameraController(); 
	cameraController_->Initialize(&camera_);    
	cameraController_->SetTarget(player_);
	cameraController_->Reset();
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
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticleModel_, &camera_, deathParticlesPosition);
		}
		// 移動回数が0のとき、Rキーでリセット
		else if (player_->GetRemainingMoves() <= 0 && Input::GetInstance()->TriggerKey(DIK_R)) {
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

			if (boxCount > 0 && allBroken && !player_->IsMoving()) {
				phase_ = Phase::kFadeOut;
				fade_->Start(Fade::Status::FadeOut, 1.0f);
			}
		}
		break;
	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
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
			// プレイヤーが死亡しているか、リセットボタンが押された場合
			else if (player_->IsDead() || player_->GetRemainingMoves() <= 0) {
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
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/floorBlocks.csv", 0);
	mapChipField_->LoadMapChipCsv("Resources/mapCsv/ravageBlocks.csv", 1);

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

	// カメラの初期化
	cameraController_->Reset();

	// 死亡エフェクトがあれば消す
	if (deathParticles_) {
		delete deathParticles_;
		deathParticles_ = nullptr;
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

			// レイヤー 1: 破壊可能なブロック (Box)
			if (mapChipField_->GetMapChipTypeByIndex(j, i, 1) == MapChipType::kBlock) {
				Box* newBox = new Box();
				Vector3 position = mapChipField_->GetMapChipPositionByIndex(j, i);

				// 高さを合わせる
				if (i < 10) {
					// 1〜10行目　1段目
					position.y = 1.0f;
				} else if (10 <= i && i < 20) {
					// 11〜20行目　2段目
					position.y = 2.0f;
					position.z += (10.0f * MapChipField::kBlockHeight);
				}
				newBox->Initialize(blockModel_, &camera_, position);
				boxes_.push_back(newBox);
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
		CheckAllCollisions();

		for (Box* box : boxes_) {
			if (!box->IsAlive())
				continue;

			Vector3 pos = box->GetPosition();

			// 1段目は落下しない
			if (pos.y <= 1.05f) {
				box->SetonGround(true);
				continue;
			}

			// 自分の1マス下の座標を出す
			Vector3 underPos = pos;
			underPos.y -= 1.0f;
			// その場所のインデックスを取得
			MapChipField::IndexSet indexUnder = mapChipField_->GetMapChipIndexSetByPosition(underPos);

			// その場所にブロックがあるか判定する
			MapChipType typeUnder = mapChipField_->GetMapChipTypeByIndex(indexUnder.xIndex, indexUnder.yIndex, 1);

			//判定
			if (typeUnder == MapChipType::kBlank) {
				// 真下が空っぽなら落下する
				box->SetonGround(false);
			} else {
				// 下にブロックがあれば着地
				box->SetonGround(true);

				// y座標を整数値に補正
				pos.y = std::round(pos.y);
				box->SetPosition(pos);
			}
		}

		for (Box* box : boxes_) {
			box->Update();
		}
		break;

	case Phase::kDeath:
		if (deathParticles_) {
			deathParticles_->Update();
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

	for (uint32_t layer = 0; layer < worldTransformBlocks_.size(); ++layer) {
		for (uint32_t i = 0; i < worldTransformBlocks_[layer].size(); ++i) {
			for (uint32_t j = 0; j < worldTransformBlocks_[layer][i].size(); ++j) {
				WorldTransform* block = worldTransformBlocks_[layer][i][j];
				if (block && mapChipField_->GetMapChipTypeByIndex(j, i, layer) != MapChipType::kBlank) {
					blockModel_->Draw(*block, camera_);
				}
			}
		}
	}

	if (player_->IsDead() && deathParticles_) {
		deathParticles_->Draw();
	}

	for (Box* box : boxes_) {
		box->Draw();
	}

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
	// エネミー削除に伴い、現在はプレイヤーとマップの判定のみ（Playerクラス内で処理済み）
	// 将来的にアイテム等の判定が必要になればここに追加する

	AABB playerAABB = player_->GetAABB();

	for (Box* box : boxes_) {
		// すでに壊れている場合はスキップ
		if (!box->IsAlive()) {
			continue;
		}

		AABB boxAABB = box->GetAABB();

		if (IsCollision(playerAABB, boxAABB)) {
				box->OnCollision();
			Vector3 pos = box->GetPosition();
			MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(pos);
			mapChipField_->SetMapChipTypeByIndex(index.xIndex, index.yIndex, MapChipType::kBlank, 1);
		}
	}
}
