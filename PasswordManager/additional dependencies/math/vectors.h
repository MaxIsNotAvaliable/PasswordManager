#pragma once
#include "vectorMath.h"

union Vector2;
union Vector3;
union Vector4;

typedef Vector2 Vec2;
typedef Vector3 Vec3;
typedef Vector4 Vec4;

union Vector2
{
private:
	typedef Vector<1> PrevDim;
	typedef Vector<3> NextDim;
	typedef Vector2 ThisClass;

public:
	Vector<2> raw = Vector<2>();
	struct
	{
		float x, y;
	};

	Vector2() {};
	Vector2(float v) : x(v), y(v) {};
	Vector2(float x, float y) : x(x), y(y) {};
	Vector2(const Vector<2>& vec) : x(vec[0]), y(vec[1]) {};

	//---------------------//
	//  OPERATOR OVERLOAD  //
	//---------------------//
#pragma region operator overload

	template<typename T>
	const ThisClass operator+(const T& other) const
	{
		return raw.operator+(other);
	}

	template<typename T>
	const ThisClass operator-(const T& other) const
	{
		return raw.operator-(other);
	}

	template<typename T>
	const ThisClass operator*(const T& other) const
	{
		return raw.operator*(other);
	}

	const ThisClass operator*(const Matrix& matrix2x2) const
	{
		return VectorMath::MatrixMultiply(this->raw, matrix2x2);
	}

	template<typename T>
	const ThisClass operator/(const T& other) const
	{
		return raw.operator/(other);
	}

	template<typename T>
	ThisClass operator+=(const T& other)
	{
		return raw.operator+=(other);
	}

	template<typename T>
	ThisClass operator-=(const T& other)
	{
		return raw.operator-=(other);
	}

	template<typename T>
	ThisClass operator*=(const T& other)
	{
		return raw.operator*=(other);
	}

	template<typename T>
	ThisClass operator/=(const T& other)
	{
		return raw.operator/=(other);
	}

	float& operator[](int i)
	{
		return this->raw[i];
	}

	const float& operator[](int i) const
	{
		return this->raw[i];
	}

	operator float() const
	{
		return this->raw.operator float();
	}

	bool operator==(const ThisClass& other)
	{
		return this->IsInRange(other, 0.01f);
	}
	bool operator!=(const ThisClass& other)
	{
		return !this->IsInRange(other, 0.01f);
	}

	operator const char* () const
	{
		char buf[0x32];
		sprintf_s(buf, "%.3f %.3f", this->x, this->y);
		return buf;
	}

#pragma endregion


	//---------------------//
	//  FUNCTIONS          //
	//---------------------//
#pragma region functions

	const float CalcAbsAngle(const ThisClass& other) const
	{
		return VectorMath::CalcAbsAngle(this->raw, other.raw);
	}

	const ThisClass GetNormalized() const noexcept
	{
		return VectorMath::NormalizedInPlace(this->raw);
	}

	const float Lenght() const noexcept
	{
		return VectorMath::Lenght(this->raw);
	}

	const float DistanceTo(const ThisClass& other) const noexcept
	{
		return VectorMath::DistanceBetween(this->raw, other.raw);
	}

	const bool IsInRange(const ThisClass& other, float radius) const noexcept
	{
		return VectorMath::IsInRange(this->raw, other.raw, radius);
	}

	const float DotProduct(const ThisClass& other) const noexcept
	{
		return VectorMath::DotProduct(this->raw, other.raw);
	}

	const float DotProductNormalized(const ThisClass& other) const noexcept
	{
		return VectorMath::DotProduct(this->GetNormalized().raw, other.GetNormalized().raw);
	}

	const Matrix ToMatrix() const noexcept
	{
		return VectorMath::ToMatrix(this->raw);
	}

	const ThisClass CalcAngles(const ThisClass& dst = { 0, 0 }) const
	{
		return VectorMath::CalcAngles(this->raw, dst.raw);
	}

#pragma region specific

	const ThisClass AngleToVector() const
	{
		return VectorMath::AngleToVector(this->raw);
	}

	static ThisClass CircleUnit(float radians)
	{
		return VectorMath::AngleToVector(radians);
	}

	NextDim ToVec3()
	{
		return this->raw.Resize<3>();
	}

	Vec2& RotateRad(float angleRad)
	{
		float oldX = this->x;
		float oldY = this->y;
		this->x = oldX * cosf(angleRad) - oldY * sinf(angleRad);
		this->y = oldX * sinf(angleRad) + oldY * cosf(angleRad);
		return *this;
	}
	Vec2& Rotate(float angleDeg)
	{
		return RotateRad(angleDeg * degToRadMul);
	}
	static Vec2 RotateRad(Vec2 vec, float angleRad)
	{
		return vec.RotateRad(angleRad);
	}
	static Vec2 Rotate(Vec2 vec, float angleDeg)
	{
		return vec.RotateRad(angleDeg * degToRadMul);
	}


	static ThisClass Lerp(const ThisClass& a, const ThisClass& b, float t)
	{
		ThisClass result = { std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t) };
		return result;
	}


#pragma endregion function

#pragma endregion
};

union Vector3
{
private:
	typedef Vector<2> PrevDim;
	typedef Vector<4> NextDim;
	typedef Vector3 ThisClass;

public:
	Vector<3> raw = Vector<3>();
	struct
	{
		float x, y, z;
	};

	Vector3() {};
	Vector3(float v) : x(v), y(v), z(v) {};
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
	Vector3(const Vector<3>& vec) : x(vec[0]), y(vec[1]), z(vec[2]) {};

	//---------------------//
	//  OPERATOR OVERLOAD  //
	//---------------------//
#pragma region operator overload

	template<typename T>
	const ThisClass operator+(const T& other) const
	{
		return raw.operator+(other);
	}

	template<typename T>
	const ThisClass operator-(const T& other) const
	{
		return raw.operator-(other);
	}

	template<typename T>
	const ThisClass operator*(const T& other) const
	{
		return raw.operator*(other);
	}

	const ThisClass operator*(const Matrix& matrix3x3) const
	{
		return VectorMath::MatrixMultiply(this->raw, matrix3x3);
	}

	template<typename T>
	const ThisClass operator/(const T& other) const
	{
		return raw.operator/(other);
	}

	template<typename T>
	ThisClass operator+=(const T& other)
	{
		return raw.operator+=(other);
	}

	template<typename T>
	ThisClass operator-=(const T& other)
	{
		return raw.operator-=(other);
	}

	template<typename T>
	ThisClass operator*=(const T& other)
	{
		return raw.operator*=(other);
	}

	template<typename T>
	ThisClass operator/=(const T& other)
	{
		return raw.operator/=(other);
	}

	float& operator[](int i)
	{
		return this->raw[i];
	}

	const float& operator[](int i) const
	{
		return this->raw[i];
	}

	operator float() const
	{
		return this->raw.operator float();
	}

	bool operator==(const ThisClass& other)
	{
		return this->IsInRange(other, 0.01f);
	}
	bool operator!=(const ThisClass& other)
	{
		return !this->IsInRange(other, 0.01f);
	}

	operator const char* () const
	{
		char buf[0x32];
		sprintf_s(buf, "%.3f %.3f %.3f", this->x, this->y, this->z);
		return buf;
	}

#pragma endregion


	//---------------------//
	//  FUNCTIONS          //
	//---------------------//
#pragma region functions

	const float CalcAbsAngle(const ThisClass& other) const
	{
		return VectorMath::CalcAbsAngle(this->raw, other.raw);
	}

	const ThisClass GetNormalized() const noexcept
	{
		return VectorMath::NormalizedInPlace(this->raw);
	}

	const float Lenght() const noexcept
	{
		return VectorMath::Lenght(this->raw);
	}

	const float DistanceTo(const ThisClass& other) const noexcept
	{
		return VectorMath::DistanceBetween(this->raw, other.raw);
	}

	const bool IsInRange(const ThisClass& other, float radius) const noexcept
	{
		return VectorMath::IsInRange(this->raw, other.raw, radius);
	}

	const float DotProduct(const ThisClass& other) const noexcept
	{
		return VectorMath::DotProduct(this->raw, other.raw);
	}

	const float DotProductNormalized(const ThisClass& other) const noexcept
	{
		return VectorMath::DotProduct(this->GetNormalized().raw, other.GetNormalized().raw);
	}


	/// <summary>
	/// Конвертирует вектор в матрицу 3x1
	/// </summary>
	/// <returns></returns>
	const Matrix ToMatrix() const noexcept
	{
		return VectorMath::ToMatrix(this->raw);
	}

	//const ThisClass CaclAngles(const ThisClass& dst = { 0, 0, 0 }) const
	//{
	//	return VectorMath::CaclAngles(this->raw, dst.raw);
	//}

	ThisClass CalcAngles(const ThisClass& dst = { 0, 0, 0 }) const
	{
		return VectorMath::CalcAngles(this->raw, dst.raw);
	}

#pragma region specific

	const ThisClass CrossProduct(const ThisClass& other) const
	{
		return VectorMath::CrossProduct(this->raw, other.raw);
	}

	Vector2 ToVec2()
	{
		return this->raw.Resize<2>();
	}

	float Lenght2d()
	{
		return this->ToVec2().Lenght();
	}

	Vector3 ToVector()
	{
		return VectorMath::AngleToVector(this->raw);
	}

	ThisClass NormalizeVector() noexcept
	{
		*this = VectorMath::NormalizedInPlace(this->raw);
		return *this;
	}

	static ThisClass LerpVec3(const ThisClass& a, const ThisClass& b, float t)
	{
		ThisClass result = { std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t), std::lerp(a.z, b.z, t) };
		return result;
	}
	static float DistanceBetween(const ThisClass& a, const ThisClass& b)
	{
		return a.DistanceTo(b);
	}

#pragma endregion function

#pragma endregion
};

union Vector4
{
private:
	typedef Vector3 PrevDim;
	typedef Vector<5> NextDim;
	typedef Vector4 ThisClass;

public:
	Vector<4> raw = Vector<4>();
	struct
	{
		float x;
		float y;
		float z;
		float w;
	};

	Vector4() {};
	Vector4(float v) : x(v), y(v), z(v), w(v) {};
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};
	Vector4(const Vector<4>& vec) : x(vec[0]), y(vec[1]), z(vec[2]), w(vec[3]) {};


	//---------------------//
	//  OPERATOR OVERLOAD  //
	//---------------------//
#pragma region operator overload

	template<typename T>
	const ThisClass operator+(const T& other) const
	{
		return raw.operator+(other);
	}

	template<typename T>
	const ThisClass operator-(const T& other) const
	{
		return raw.operator-(other);
	}

	template<typename T>
	const ThisClass operator*(const T& other) const
	{
		return raw.operator*(other);
	}

	const ThisClass operator*(const Matrix& matrix4x4) const
	{
		return VectorMath::MatrixMultiply(this->raw, matrix4x4);
	}

	template<typename T>
	const ThisClass operator/(const T& other) const
	{
		return raw.operator/(other);
	}

	template<typename T>
	ThisClass operator+=(const T& other)
	{
		return raw.operator+=(other);
	}

	template<typename T>
	ThisClass operator-=(const T& other)
	{
		return raw.operator-=(other);
	}

	template<typename T>
	ThisClass operator*=(const T& other)
	{
		return raw.operator*=(other);
	}

	template<typename T>
	ThisClass operator/=(const T& other)
	{
		return raw.operator/=(other);
	}

	float& operator[](int i)
	{
		return this->raw[i];
	}

	const float& operator[](int i) const
	{
		return this->raw[i];
	}

	operator float() const
	{
		return this->raw.operator float();
	}

	operator const char* () const
	{
		char buf[0x32];
		sprintf_s(buf, "%.3f %.3f %.3f %.3f", this->x, this->y, this->z, this->w);
		return buf;
	}

	bool operator==(const ThisClass& other)
	{
		return this->IsInRange(other, 0.01f);
	}
	bool operator!=(const ThisClass& other)
	{
		return !this->IsInRange(other, 0.01f);
	}

#pragma endregion


	//---------------------//
	//  FUNCTIONS          //
	//---------------------//
#pragma region functions

	const float CalcAbsAngle(const ThisClass& other) const
	{
		return VectorMath::CalcAbsAngle(this->raw, other.raw);
	}

	const ThisClass GetNormalized() const noexcept
	{
		return VectorMath::NormalizedInPlace(this->raw);
	}

	const float Lenght() const noexcept
	{
		return VectorMath::Lenght(this->raw);
	}

	const float DistanceTo(const ThisClass& other) const noexcept
	{
		return VectorMath::DistanceBetween(this->raw, other.raw);
	}

	const bool IsInRange(const ThisClass& other, float radius) const noexcept
	{
		return VectorMath::IsInRange(this->raw, other.raw, radius);
	}

	const float DotProduct(const ThisClass& other) const noexcept
	{
		return VectorMath::DotProduct(this->raw, other.raw);
	}

	const float DotProductNormalized(const ThisClass& other) const noexcept
	{
		return VectorMath::DotProduct(this->GetNormalized().raw, other.GetNormalized().raw);
	}

	const Matrix ToMatrix() const noexcept
	{
		return VectorMath::ToMatrix(this->raw);
	}

	//const ThisClass CalcAngles(const ThisClass& dst = { 0, 0, 0, 0 }) const
	//{
	//	return VectorMath::CalcAngles(this->raw, dst.raw);
	//}

#pragma region specific

	// none

#pragma endregion function

#pragma endregion
};

