#pragma once
#include <concepts>
#include <iostream>
#include "Component.h"

namespace ThornEngine {

struct Transform : IComponent {
	Transform() { *this = Identity(); }
	V2F pos, scale;
	Transform(float pX, float pY, float sX = 1, float sY = 1) : pos(V2F(pX, pY)), scale(V2F(sX, sY)) {}
	Transform(V2F newPos, V2F newScale = V2F::One) : pos(newPos), scale(newScale) {}
	friend std::ostream& operator<<(std::ostream& os, const Transform& transform) {
		os << "TForm[pos: " << transform.pos << ", scale: " << transform.scale << ']';
		return os;
	}
	static const Transform Identity() { return Transform(0, 0, 1, 1); }
};

using TForm = Transform;

}