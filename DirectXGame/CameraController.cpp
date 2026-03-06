#include <algorithm>
#include "CameraController.h"
#include "Math.h"
#include "Player.h"


Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
	Vector3 result;
	result.x = start.x + (end.x - start.x) * t;
	result.y = start.y + (end.y - start.y) * t;
	result.z = start.z + (end.z - start.z) * t;
	return result;
}

void CameraController::Initialize(Camera* camera) { 
	camera_ = camera;
}

void CameraController::Update() {
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	const Vector3& targetVelocity = target_->GetVelocity();
	
	destination_.x = targetWorldTransform.translation_.x + targetOffset_.x + targetVelocity.x * kVelocityBias;
	destination_.y = targetWorldTransform.translation_.y + targetOffset_.y + targetVelocity.y * kVelocityBias;
	destination_.z = targetWorldTransform.translation_.z + targetOffset_.z + targetVelocity.z * kVelocityBias;

	camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);



	camera_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
	camera_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
	camera_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;

	camera_->translation_.x = max(camera_->translation_.x, destination_.x + targetMargin.left);
	camera_->translation_.x = min(camera_->translation_.x, destination_.x + targetMargin.right);
	camera_->translation_.y = max(camera_->translation_.y, destination_.y + targetMargin.bottom);
	camera_->translation_.y = min(camera_->translation_.y, destination_.y + targetMargin.top);


	camera_->translation_.x = max(camera_->translation_.x, movableArea_.left);
	camera_->translation_.x = min(camera_->translation_.x, movableArea_.right);
	camera_->translation_.y = min(camera_->translation_.y, movableArea_.bottom);
	camera_->translation_.y = max(camera_->translation_.y, movableArea_.top);


	camera_->UpdateMatrix();
}


void CameraController::Reset() { 
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	camera_->translation_ = {targetWorldTransform.translation_.x + targetOffset_.x, targetWorldTransform.translation_.y + targetOffset_.y, targetWorldTransform.translation_.z + targetOffset_.z};
}
