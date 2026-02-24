#pragma once
#include <concepts>
#include <iostream>
#include "Component.h"

template<typename T>
concept VectorConcept = std::is_same_v<T, int> || std::is_same_v<T, float>;

template<VectorConcept T>
struct V2 {
	V2() = default;
	T x, y;
	V2(T newX, T newY) : x(newX), y(newY) {}
	friend std::ostream& operator<<(std::ostream& os, const V2<T>& vec) {
		char prefix = std::is_same_v<T, int> ? 'I' : 'F';
		os << prefix << "(" << vec.x << ", " << vec.y << ")";
		return os;
	}
};

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
	inline static Transform Identity() { return Transform(0, 0, 1, 1); }
};