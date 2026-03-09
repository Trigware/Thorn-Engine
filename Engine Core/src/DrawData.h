#pragma once
#include "Component.h"
#include "Assets.h"

class DrawData : IComponent {
public:
	DrawData() = default;
	DrawData(SceneContext* context) : sceneContext(context) {}
	template<Enum T>
	void ReplaceTexture(T identifier) { textureAsset = sceneContext->appContext->GetAsset<TextureRes>(identifier); }
	void RemoveTexture() { textureAsset.texture = nullptr; }
	V2I tileIndex;
	bool isCentered = false;
private:
	friend class Sprite;
	Texture textureAsset;
	SceneContext* sceneContext = nullptr;
};