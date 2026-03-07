#include "Sprite.h"

Sprite::Sprite(SceneContext* context) {
	sceneContext = context;
	drawData = DrawData(context);
}

void Sprite::OnDraw() {
	if (drawData.textureAsset.texture == nullptr) return;

	V2I& tileSize = drawData.textureAsset.tileSize;
	V2I displayedSize = tileSize * transform.scale;

	SDL_Rect screenRect = {transform.pos.x, transform.pos.y, displayedSize.x, displayedSize.y},
		sourceRect = {drawData.tileIndex.x, drawData.tileIndex.y, tileSize.x, tileSize.y};

	SDL_RenderCopy(sceneContext->appContext->renderer, drawData.textureAsset.texture, &sourceRect, &screenRect);
}