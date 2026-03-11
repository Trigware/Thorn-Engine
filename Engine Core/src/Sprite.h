#pragma once
#include "Component.h"

struct Sprite : IActorType {
	inline Sprite(Actor& owner) { owner.BindMore<Transform, DrawData>(); }
	template<Enum T>
	inline Sprite(Actor& owner, T identifier, V2F position = V2F::Zero, V2F scale = V2F::Zero) {
		owner.Bind<Transform>(position, scale).Bind<DrawData>(identifier);
	}
};