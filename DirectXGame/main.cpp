#include "KamataEngine.h"
#include <Windows.h>
#include"GameScene.h"
#include "TitleScene.h" 

using namespace KamataEngine;

TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;

enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
};

Scene scene = Scene::kUnknown;

void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		// 02_12 30枚目
		if (gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			delete gameScene;
			gameScene = nullptr;
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene() {

	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	using namespace KamataEngine;
	// エンジンの初期化
	KamataEngine::Initialize(L"TD3_L3 ");
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	
	scene = Scene::kTitle;
	titleScene = new TitleScene;
	titleScene->Initialize();

	while (true) {
	//エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}
		// シーン切り替え
		ChangeScene();

		// シーン更新
		UpdateScene();

		//描画開始
		dxCommon->PreDraw();

		// シーンの描画
		DrawScene();

		// 軸表示の描画
		AxisIndicator::GetInstance()->Draw();

		// プリミティブ描画のリセット
		PrimitiveDrawer::GetInstance()->Reset();

		//描画終了
		dxCommon->PostDraw();
	}

	// ゲームシーンの解放
	delete titleScene;
	delete gameScene;
	
	
	//エンジンの終了処理
	KamataEngine::Finalize();
	return 0;
}
