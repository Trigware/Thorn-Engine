#pragma once
#include "Actor.h"
#include "Component.h"
#include "Assets.h"
#include "DrawData.h"

class Sprite : public IActorType {
public:
	Sprite(SceneContext* context);
	void OnDraw() override;
	template<Enum T>
	Sprite(SceneContext* context, T identifier) { sceneContext = context; drawData = DrawData(context); drawData.ReplaceTexture(identifier); }
private:
	Transform transform;
	DrawData drawData;
	ComponentMap GetComponents() { return {
		{typeid(Transform), &transform},
		{typeid(DrawData), &drawData}
	};}
};