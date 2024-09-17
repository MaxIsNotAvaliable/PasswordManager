#pragma once

#include <numbers>
#include "Matrix.h"
#include <initializer_list>

#ifndef MATH_NUMS
#define MATH_NUMS
static const float pi = std::numbers::pi_v<float>;	// 3.14...
static const float piHalf = pi / 2;					// pi / 2
static const float piTwo = pi * 2;					// pi * 2
static const float radToDegMul = 180.f / pi;		// this * angle_rad
static const float degToRadMul = pi / 180.f;		// this * angle_deg
#endif // !MATH_NUMS


template<typename T>
constexpr T GETSIGN(const T& value) { return abs(value) / value; }


using std::lerp;

#ifdef max

template<typename T>
constexpr T MAX(const T& a, const T& b){return max(a, b);}
#else
template<typename T>
constexpr T MAX(const T& a, const T& b){return std::max(a, b);}
#endif

#ifdef min
template<typename T>
constexpr T MIN(const T& a, const T& b){return min(a, b);}
#else
template<typename T>
constexpr T MIN(const T& a, const T& b){return std::min(a, b);}
#endif

template<typename T>
constexpr T CLAMP(const T& left, const T& right, const T& value){return MIN(right, MAX(left, value));}

constexpr float minf(float a, float b)
{
	return MIN(a, b);
}
constexpr float maxf(float a, float b)
{
	return MAX(a, b);
}
constexpr float lerpf(float a, float b, float t)
{
	return lerp(a, b, t);
}
constexpr float clampf(float left, float right, float value)
{
	return CLAMP(left, right, value);
}

template <size_t Dimension>
class Vector
{
protected:
	void InitList(const std::initializer_list<float>& list)
	{
		size_t i = 0;
		for (const float* it = list.begin(); it != list.end() && i < Dimension; ++it, ++i)
		{
			data[i] = *it;
		}
	}

	template<typename T>
	void InitValue(T value)
	{
		size_t i = 0;
		for (size_t i = 0; i < Dimension; i++)
		{
			data[i] = (float)value;
		}
	}

	template<typename T>
	static float GetValue(const T& value, size_t i, char* pMemoizeSame = nullptr)
	{
		static constexpr char MEM_SAME_TRUE = (char)0x01;
		static constexpr char MEM_SAME_FALSE = (char)0x02;

		if (pMemoizeSame)
		{
			if (!*pMemoizeSame)
			{
				bool sameSize = sizeof(T) == sizeof(Vector<Dimension>);
				*pMemoizeSame = sameSize ? MEM_SAME_TRUE : MEM_SAME_FALSE;
			}

			//if (std::is_class<Vector>::value) {}

			return *pMemoizeSame == MEM_SAME_TRUE ? (*(Vector<Dimension>*)(&value))[i] : (float)(value);
		}

		static_assert(sizeof(T) == sizeof(Vector<Dimension>) || sizeof(T) == sizeof(float), "Unable to parse value");
		return sizeof(T) == sizeof(Vector<Dimension>) ? (*(Vector<Dimension>*)(&value))[i] : (float)(value);
	}

public:
	float data[Dimension] = { 0 };

	Vector()
	{
		static_assert(sizeof(Vector<Dimension>) == sizeof(float) * Dimension, "Vector<D> should match float array size");
	}
	Vector(const std::initializer_list<float>& list)
	{
		static_assert(sizeof(Vector<Dimension>) == sizeof(float) * Dimension, "Vector<D> should match float array size");
		InitList(list);
	}

	explicit Vector(float value)
	{
		InitValue(value);
	}
	explicit Vector(size_t value)
	{
		InitValue(value);
	}
	explicit Vector(double value)
	{
		InitValue(value);
	}

	template<size_t D>
	const Vector<D> Resize(float fillValues = 0)
	{
		Vector<D> vec = Vector<D>(fillValues);
		for (size_t i = 0; i < MIN(D, Dimension); i++)
		{
			vec.data[i] = this->data[i];
		}
		return vec;
	}

	template<typename T>
	const Vector operator+(const T& other) const noexcept
	{
		Vector v;
		char sameSize = 0;
		for (size_t i = 0; i < Dimension; i++)
		{
			v.data[i] = this->data[i] + GetValue<T>(other, i, &sameSize);
		}
		return v;
	}

	template<typename T>
	const Vector operator-(const T& other) const noexcept
	{
		Vector v;
		char sameSize = 0;
		for (size_t i = 0; i < Dimension; i++)
		{
			v.data[i] = this->data[i] - GetValue<T>(other, i, &sameSize);
		}
		return v;
	}

	template<typename T>
	const Vector operator*(const T& other) const noexcept
	{
		Vector v;
		char sameSize = 0;
		for (size_t i = 0; i < Dimension; i++)
		{
			v.data[i] = this->data[i] * GetValue<T>(other, i, &sameSize);
		}
		return v;
	}

	template<typename T>
	const Vector operator/(const T& other) const noexcept
	{
		Vector v;
		char sameSize = 0;
		for (size_t i = 0; i < Dimension; i++)
		{
			v.data[i] = this->data[i] / GetValue<T>(other, i, &sameSize);
		}
		return v;
	}


	template<typename T>
	const Vector& operator+=(const T& other)
	{
		static_assert((sizeof(T) == sizeof(Vector) || sizeof(T) == sizeof(float)), "`Vector += (not vector)` is not allowed");
		*this = *this + other;
		return *this;
	}

	template<typename T>
	const Vector& operator-=(const T& other)
	{
		static_assert((sizeof(T) == sizeof(Vector) || sizeof(T) == sizeof(float)), "`Vector -= (not vector)` is not allowed");
		*this = *this - other;
		return *this;
	}

	template<typename T>
	const Vector& operator*=(const T& other)
	{
		static_assert((sizeof(T) == sizeof(Vector) || sizeof(T) == sizeof(float)), "`Vector *= (not vector)` is not allowed");
		*this = *this * other;
		return *this;
	}

	template<typename T>
	const Vector& operator/=(const T& other)
	{
		static_assert((sizeof(T) == sizeof(Vector) || sizeof(T) == sizeof(float)), "`Vector /= (not vector)` is not allowed");
		*this = *this / other;
		return *this;
	}

	const Vector operator*(const Matrix& matrix) const noexcept
	{
		Vector vec;
		for (size_t i = 0; i < Dimension; i++)
		{
			float line = 0;
			for (size_t j = 0; j < Dimension; j++)
			{
				line += matrix[j][i] * this->data[j];
			}
			vec[i] = line;
		}
		return vec;
	}

	float& operator[](size_t i)
	{
		return this->data[i];
	}
	const float& operator[](size_t i) const
	{
		return this->data[i];
	}

	// No no no, you can't do that
	operator float() const
	{
		//throw std::exception("No no no, you can't do that.");
		//void* pf = _ReturnAddress();
		return FLT_MIN;
		//return this->data[0];
	}

	const bool operator==(const Vector& other) const
	{
		return std::memcmp(this->data, other.data, sizeof(this->data));
	}
};

class VectorMath
{


public:
	template<size_t D = 1>
	static const float DotProduct(const Vector<D>& vecA, const Vector<D>& vecB) noexcept
	{
		float product = 0;
		for (size_t i = 0; i < D; i++)
		{
			product += vecA[i] * vecB[i];
		}
		return product;
	}

	template<size_t D = 1>
	static const Matrix ToMatrix(const Vector<D>& vec) noexcept
	{
		Matrix m = Matrix(D, 1);
		for (size_t i = 0; i < D; i++)
		{
			m[i][0] = vec[i];
		}
		return m;
	}

	template<size_t D = 1>
	static const float CalcAbsAngle(const Vector<D>& vecA, const Vector<D>& vecB) {
		float dot_product = VectorMath::DotProduct<D>(vecA, vecB);
		float normal = VectorMath::Lenght(vecA) * VectorMath::Lenght(vecB);

		return acosf(dot_product / normal) * radToDegMul;
	}

	static const float CalcAbsAngle(const Vector<2>& vec) {
		return atan2f(vec[1], vec[0]) * radToDegMul;
	}

	template<size_t D = 1>
	static const Vector<D> MatrixMultiply(const Vector<D>& vector, const Matrix& matrixDxD) noexcept
	{
		//if (matrixDxD.Cols() != matrixDxD.Rows() || matrixDxD.Cols() != D)
		//{
		//	throw std::exception("Can not multiply vector by matrix");
		//}

		Vector<D> vec;
		for (size_t i = 0; i < D; i++)
		{
			float line = 0;
			for (size_t j = 0; j < D; j++)
			{
				line += matrixDxD[j][i] * vector.data[j];
			}
			vec[i] = line;
		}
		return vec;
	}

	static const Vector<3> CrossProduct(const Vector<3>& vecA, const Vector<3>& vecB) noexcept
	{
		Vector<3> cross;

		size_t sz = sizeof(vecA.data) / sizeof(vecA.data[0]);
		for (size_t i = 0; i < sz; i++)
		{
			size_t j = (i + 1) % sz;
			size_t k = (j + 1) % sz;
			cross[i] = vecA[j] * vecB[k] - vecA[k] * vecB[j];
		}
		return cross;
	}

	static const Vector<3> AngleToVector(const Vector<3>& angle)
	{
		float pitch = angle[0] * degToRadMul;
		float yaw = angle[1] * degToRadMul;

		Vector<3> vec;
		vec[0] = cosf(pitch) * cosf(yaw);
		vec[1] = cosf(pitch) * sinf(yaw);
		vec[2] = -sinf(pitch);

		return vec;
	}

	static const Vector<2> AngleToVector(const Vector<1>& angle)
	{
		float rad = angle[0] * degToRadMul;
		return Vector<2>{ cosf(rad), sinf(rad)};
	}

	static const Vector<2> AngleToVector(float angle) {
		return VectorMath::AngleToVector(Vector<1>{ angle });
	}

	static const Vector<2> CalcAngles(Vector<2> src, Vector<2> dst = { 0 })
	{
		Vector<2> deltaVec = dst - src;
		Vector<2> angle;
		angle[0] = atan2f(deltaVec[1], deltaVec[0]);
		return angle * radToDegMul;
	}
	static const Vector<3> CalcAngles(Vector<3> src, Vector<3> dst = { 0 })
	{
		Vector<3> deltaVec = dst - src;
		float len = VectorMath::Lenght(deltaVec.Resize<2>());

		Vector<3> angle = {
			atan2(-deltaVec[2], len),
			atan2(deltaVec[1], deltaVec[0]),
			0
		};

		return angle * radToDegMul;
	}

	//template<size_t D = 1>
	//static const Vector<D> CalcAngles(Vector<D> src, Vector<D> dst = { 0 })
	//{
	//	Vector<D> deltaVec = dst - src;

	//	float len = VectorMath::Lenght(deltaVec);

	//	Vector<D> angle;

	//	if (D == 2)
	//	{
	//		angle[0] = atan2f(deltaVec[1], deltaVec[0]);
	//		return angle * radToDegMul;
	//	}
	//	else if (D == 3)
	//	{
	//		float ah = sqrt(deltaVec[0] * deltaVec[0] + deltaVec[1] * deltaVec[1]);
	//		angle[0] = atan2(-deltaVec[2], ah);
	//		angle[1] = atan2(deltaVec[1], deltaVec[0]);
	//		angle[2] = 0.0f;

	//		return angle * radToDegMul;
	//	}


	//	angle[0] = atan2(-deltaVec[2], len);
	//	angle[1] = atan2(deltaVec[1], deltaVec[0]);

	//	return angle * radToDegMul;
	//}

	template<size_t D = 1>
	static const Vector<D> NormalizedInPlace(const Vector<D>& vec) noexcept
	{
		Vector<D> v;
		float iradius = 1.f / (VectorMath::Lenght(vec) + FLT_EPSILON);
		for (size_t i = 0; i < D; i++)
		{
			v.data[i] = vec.data[i] * iradius;
		}
		return v;
	}

	template<size_t D = 1>
	static const float Lenght(const Vector<D>& vec) noexcept
	{
		float sum = 0;
		for (size_t i = 0; i < D; i++)
		{
			sum += powf(vec.data[i], 2);
		}
		return sqrtf(sum);
	}

	template<size_t D = 1>
	static const float DistanceBetween(const Vector<D>& vecA, const Vector<D>& vecB) noexcept
	{
		return VectorMath::Lenght(Vector<D>(vecA - vecB));
	}

	template<size_t D = 1>
	static const bool IsInRange(const Vector<D>& vecA, const Vector<D>& vecB, float radius) noexcept
	{
		return DistanceBetween(vecA, vecB) < radius;
	}

	template<size_t D = 1>
	static const Vector<D> Lerp(const Vector<D>& vecA, const Vector<D>& vecB, float t) noexcept
	{
		Vector<D> v;
		for (size_t i = 0; i < D; i++)
		{
			v[i] = std::lerp(vecA[i], vecB[i], t);
		}
		return v;
	}


};
