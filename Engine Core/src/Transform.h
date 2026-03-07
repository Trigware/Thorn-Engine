#pragma once
#include <concepts>
#include <iostream>
#include "Component.h"

template<typename T>
concept VectorConcept = std::is_same_v<T, int> || std::is_same_v<T, float>;

template<VectorConcept T>
struct V2 {
	T x, y;
	V2() : x(0), y(0) {}
	V2(T newX, T newY) : x(newX), y(newY) {}
	constexpr explicit V2(T value) : x(value), y(value) {}
	static const V2 Zero, One, Left, Right, Up, Down;
	V2 operator*(const V2& other) { return V2(x * other.x, y * other.y); }

	template<typename T2>
	V2(const V2<T2>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

	friend std::ostream& operator<<(std::ostream& os, const V2<T>& vec) {
		char prefix = std::is_same_v<T, int> ? 'I' : 'F';
		os << "V2" << prefix << "(" << vec.x << ", " << vec.y << ")";
		return os;
	}
};

#define VecProperty template<VectorConcept T> const V2<T> V2<T>

VecProperty::Zero{0,0}; VecProperty::One{1,1};
VecProperty::Left{1,0}; VecProperty::Right{-1,0}; VecProperty::Up{0,1}; VecProperty::Down{0,-1};

using V2I = V2<int>;
using V2F = V2<float>;

struct Transform : IComponent {
	Transform() { *this = Identity(); }
	V2F pos, scale;
	Transform(float pX, float pY, float sX, float sY) : pos(V2F(pX, pY)), scale(V2F(sX, sY)) {}
	friend std::ostream& operator<<(std::ostream& os, const Transform& transform) {
		os << "T[pos: " << transform.pos << ", scale: " << transform.scale << ']';
		return os;
	}
	static const Transform Identity() { return Transform(0, 0, 1, 1); }
};