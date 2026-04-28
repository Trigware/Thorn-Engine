#include "DrawData.h"
#include "Actor.h"
#include "AppContext.h"

namespace ThornEngine {

void DrawData::OnDraw() {
	bool hidingTexture = textureAsset == nullptr;
	if (hidingTexture) return;

	Actor owner = GetOwner();
	Transform& transform = owner.Get<Transform>();
	V2I displayedSize = textureAsset->tileSize * transform.scale,
		topLeftPos = displayedSize * tileIndex;

	SDL_Rect dstRect = {transform.pos.x, transform.pos.y, displayedSize.x, displayedSize.y},
		srcRect = {topLeftPos.x, topLeftPos.y, displayedSize.x, displayedSize.y};

	SDL_RenderCopy(sceneContext->appContext->renderer, textureAsset->texture, &srcRect, &dstRect);
}

}