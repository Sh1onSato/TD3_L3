#include "Interface.h"

void Interface::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const Vector3& position) {
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	// GameSceneから渡された {-6.0f, 4.5f, 5.0f} などのオフセットをそのまま保存
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
}

void Interface::Update() {}

void Interface::Draw(int remainingMoves) {
	if (!model_ || !camera_)
		return;

	// GameScene側で設定した { -6.0f, 4.5f, 5.0f } の値を取得
	Vector3 offset = worldTransform_.translation_;

	// カメラの現在の位置を取得
	Vector3 cameraPos = camera_->translation_;

	// player.cpp の remainingMoves_ の数に合わせてループ
	for (int i = 0; i < remainingMoves; ++i) {

		// ★CameraControllerの仕様（俯瞰・追従）に完全に合わせた座標計算
		// 1. X座標：カメラの位置を基準に、offset.x 分だけ左にずらし、個数分(i)だけ右に並べる
		worldTransform_.translation_.x = cameraPos.x + offset.x + (i * 0.4f);

		// 2. Y座標とZ座標：カメラは斜め上から見下ろしている（Y=15, Z=-15付近）ため、
		//    単純に足すとカメラの頭上に配置されてしまいます。
		//    カメラの視線（斜め下）に合わせて、Yを下げて、Zを前に出すことで「画面の左上」に固定します。
		worldTransform_.translation_.y = cameraPos.y - 4.0f + offset.y; // カメラより少し下
		worldTransform_.translation_.z = cameraPos.z + 5.0f + offset.z; // カメラより少し前（プレイヤー側）

		// 描画（UpdateMatrixがない環境なのでこのままDraw）
		model_->Draw(worldTransform_, *camera_);
	}

	// 元に戻す
	worldTransform_.translation_ = offset;
}