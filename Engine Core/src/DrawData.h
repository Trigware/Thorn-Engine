#pragma once
#include "Component.h"
#include "Assets.h"

class DrawData : public IComponent {
public:
	DrawData() = default;
	DrawData(SceneContext* context) : sceneContext(context) {}
	template<Enum T>
	DrawData(SceneContext* context, T identifier) : sceneContext(context) {
		ReplaceTexture(identifier);
	}
	template<Enum T>
	void ReplaceTexture(T identifier) {
		textureAsset = sceneContext->appContext->GetAsset<TextureRes>(identifier);
	}
	inline void RemoveTexture() { textureAsset = nullptr; }
	V2I tileIndex;
	bool isCentered = false;
	void OnDraw() override;
private:
	Texture* textureAsset = nullptr;
	SceneContext* sceneContext = nullptr;
};