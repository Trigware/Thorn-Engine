#pragma once
#include "Component.h"
#include "Assets.h"

namespace ThornEngine {

class DrawData : public IComponent {
public:
	DrawData() = default;
	void OnDraw() override;
	template<Enum T> DrawData(SceneContext* context, T identifier) : IComponent(context) { ReplaceTexture(identifier); }
	template<Enum T> void ReplaceTexture(T identifier) { textureAsset = GetAsset<TextureRes>(identifier); }
	inline void RemoveTexture() { textureAsset = nullptr; }
	V2I tileIndex;
	bool isCentered = false;
private:
	Texture* textureAsset = nullptr;
};

}