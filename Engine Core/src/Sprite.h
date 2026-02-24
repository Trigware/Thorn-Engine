#pragma once
#include "Actor.h"

class Sprite : public IActorType {
public:
	Sprite() = default;
	void OnDraw() override;
private:
	Transform transform;
	ComponentMap GetComponents() { return {
		{typeid(Transform), &transform}
	};}
};