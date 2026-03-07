#pragma once
#include "Component.h"
#include "Assets.h"

class DrawData : IComponent {
public:
	DrawData() = default;
	DrawData(AppContext* context) : appContext(context) {}
	template<Enum T>
	void ReplaceTexture(T identifier) { textureAsset = appContext->GetAsset<TextureRes>(identifier); }
	void RemoveTexture() { textureAsset.texture = nullptr; }
	V2I tileIndex;
private:
	friend class Sprite;
	Texture textureAsset;
	AppContext* appContext = nullptr;
};