#pragma once
#include"KamataEngine.h"
using namespace KamataEngine;

struct AABB {
	Vector3 min;
	Vector3 max;
};

Matrix4x4 Multiply(const Matrix4x4& a, const Matrix4x4& b);
Matrix4x4 MakeScaleMatrix(const Vector3& Scale);
Matrix4x4 MakeTranslationMatrix(const Vector3& Translate);
Matrix4x4 MakeRotationXMatrix(float radian);
Matrix4x4 MakeRotationYMatrix(float radian);
Matrix4x4 MakeRotationZMatrix(float radian);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rot, const Vector3& translate);

void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform);

Vector3& operator+=(Vector3& lhs, const Vector3& rhv);
Vector3& operator-=(Vector3& lhs, const Vector3& rhv);
Vector3& operator*=(Vector3& v, float s);
Vector3& operator/=(Vector3& v, float s);

// 2項演算子オーバーロード
Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v1, const Vector3& v2);

// 代入演算子オーバーロード
Matrix4x4& operator*=(Matrix4x4& lhm, const Matrix4x4& rhm);

// 2項演算子オーバーロード
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2);

bool IsCollision(const AABB& aabb1, const AABB& aabb2);

Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);