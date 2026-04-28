#pragma once
#include "Component.h"

namespace ThornEngine {

/* @brief Actor type that binds the Transform and DrawData components to an actor.
* 
* Used for displaying single 2D images onto the screen that have attributes such as position and scale.
*/
struct Sprite : IActorType {
	inline Sprite(Actor owner) { owner.BindMore<Transform, DrawData>(); }
	template<Enum T>
	inline Sprite(Actor owner, T identifier, V2F position = V2F::Zero, V2F scale = V2F::One) {
		owner.Bind<Transform>(position, scale).Bind<DrawData>(identifier);
	}
	ComponentSet GetSet() override { return CompSet<Transform, DrawData>(); }
};

}