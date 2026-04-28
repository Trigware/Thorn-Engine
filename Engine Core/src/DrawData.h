#pragma once
#include "Component.h"
#include "Assets.h"

namespace ThornEngine {

/* @brief Component used for displaying single 2D images onto the window.
* 
* Allows working with spritesheets and can display a small section described by the texture's resource properties.
* 
* Requires the Transform component.
*/
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