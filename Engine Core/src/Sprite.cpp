#include "Sprite.h"

Sprite::Sprite(SceneContext* context) {
	sceneContext = context;
	drawData = DrawData(context);
}

void Sprite::OnDraw() {
	if (drawData.textureAsset.texture == nullptr) return;

	V2I& tileSize = drawData.textureAsset.tileSize;
	V2I displayedSize = tileSize * transform.scale, upperLeftSrcPos = drawData.tileIndex * tileSize,
		displayedPos = transform.pos;
	if (drawData.isCentered) displayedPos -= tileSize / 2;

	SDL_Rect screenRect = {displayedPos.x, displayedPos.y, displayedSize.x, displayedSize.y},
		sourceRect = {upperLeftSrcPos.x, upperLeftSrcPos.y, tileSize.x, tileSize.y};

	SDL_RenderCopy(sceneContext->appContext->renderer, drawData.textureAsset.texture, &sourceRect, &screenRect);
}