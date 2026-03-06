#include "DeathParticles.h"
void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position) {
	model_ = model;
	camera_ = camera;

	for (auto& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
	}

	objectColor_.Initialize();

	color_ = {1, 1, 1, 1};
}

void DeathParticles::Update() {

	if (isFinished_) {
		return;
	}
	counter_ += 1.0f / 60.0f;

	if (counter_ >= kDuration_) {
		counter_ = kDuration_;
		// 終了扱いにする
		isFinished_ = true;
	}

	for (uint32_t i = 0; i < kNumParticles; ++i) {
		Vector3 velocity = {kSpeed_, 0, 0};

		float angle = kAngleUnit_ * i;

		Matrix4x4 matrixRotation = MakeRotationZMatrix(angle);

		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = Transform(velocity, matrixRotation);

		// 移動処理
		worldTransforms_[i].translation_ += velocity;
	}

	color_.w = std::clamp(1.0f - counter_ / kDuration_, 0.0f, 1.0f);
	objectColor_.SetColor(color_);

	for (auto& worldTransform : worldTransforms_) {
		// ワールド行列更新（アフィン変換～DirectXに転送）
		WorldTransformUpdate(worldTransform);
	}
}

void DeathParticles::Draw() {
	if (isFinished_) {
		return;
	}

	for (auto& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}