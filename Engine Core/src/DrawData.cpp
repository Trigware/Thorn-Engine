#include "DrawData.h"
#include "Actor.h"

void DrawData::OnDraw() {
	Actor owner(sceneContext, linkedActorUUID);
	if (!owner.Has<Transform>()) throw std::runtime_error("An actor with a DrawData component must be binded with a Transform component!");
	bool textureInvalid = textureAsset == nullptr;
	if (textureInvalid) return;

	Transform& transform = owner.Get<Transform>();
	V2I displayedSize = textureAsset->tileSize * transform.scale,
		topLeftPos = displayedSize * tileIndex;

	SDL_Rect dstRect = {transform.pos.x, transform.pos.y, displayedSize.x, displayedSize.y},
		srcRect = {topLeftPos.x, topLeftPos.y, displayedSize.x, displayedSize.y};
	SDL_RenderCopy(sceneContext->appContext->renderer, textureAsset->texture, &srcRect, &dstRect);
}