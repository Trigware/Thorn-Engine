#pragma once
#include <concepts>

namespace ThornEngine {

template<typename T>
concept VectorConcept = std::is_same_v<T, int> || std::is_same_v<T, float>;

/* @brief Two-component vector usually representing a direction, position or scale of an object in 2D space.
* @tparam T Type of value stored in both components (only int or float). Can use V2I or V2F for int and float vectors instead.
*/
template<VectorConcept T>
struct V2 {
	T x, y;
	V2() : x(0), y(0) {}
	V2(T newX, T newY) : x(newX), y(newY) {}
	constexpr explicit V2(T value) : x(value), y(value) {}
	static const V2 Zero, One, Left, Right, Up, Down;
	V2 operator*(const V2& other) const { return V2(x * other.x, y * other.y); }
	V2 operator-(const V2& other) const { return V2(x - other.x, y - other.y); }
	V2 operator*(float scalar) const { return V2(x * scalar, y * scalar); }
	V2 operator/(float scalar) const { return V2(x / scalar, y / scalar); }
	V2 operator+=(const V2& other) { x += other.x; y += other.y; return *this; }
	V2 operator-=(const V2& other) { x -= other.x; y -= other.y; return *this; }
	[[nodiscard]] float Magnitude() const { return sqrt(x * x + y * y); }
	[[nodiscard]] V2<float> GetNormalized() const {
		float magnitude = Magnitude();
		if (magnitude == 0) return V2<float>::Zero;
		return *this / magnitude;
	}
	void SetNormalize() { *this = GetNormalized(); }

	template<typename T2>
	V2(const V2<T2>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

	friend std::ostream& operator<<(std::ostream& os, const V2<T>& vec) {
		char prefix = std::is_same_v<T, int> ? 'I' : 'F';
		os << "V2" << prefix << "(" << vec.x << ", " << vec.y << ")";
		return os;
	}
};

#define VEC_PROPERTY template<VectorConcept T> const V2<T> V2<T>

VEC_PROPERTY::Zero{0,0}; VEC_PROPERTY::One{1,1};
VEC_PROPERTY::Left{1,0}; VEC_PROPERTY::Right{-1,0}; VEC_PROPERTY::Up{0,1}; VEC_PROPERTY::Down{0,-1};

#undef VEC_PROPERTY

/// @brief Integer specification for a 2-dimensional vector.
using V2I = V2<int>;
/// @brief Floating-point number specification for a 2-dimensional vector.
using V2F = V2<float>;

}