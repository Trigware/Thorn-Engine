#include "Scene.h"

void SceneManager::ExecuteUpdateLoop() {
	windowRunning = true;
	while (windowRunning) {
		HandleEvents();
		SDL_SetRenderDrawColor(appContext->renderer, 0, 0, 0, 255);
		SDL_RenderClear(appContext->renderer);
		activeScene->DrawScene();
		activeScene->OnDraw();
		SDL_RenderPresent(appContext->renderer);
	}
}

void SceneManager::HandleEvents() {
	SDL_Event currentEvent;
	while (SDL_PollEvent(&currentEvent)) {
		switch (currentEvent.type) {
			case SDL_QUIT: windowRunning = false; break;
		}
	}
}

void IScene::DrawScene() {
	for (auto it = sceneContext.sceneActors.begin(); it != sceneContext.sceneActors.end(); it++) {
		std::unique_ptr<IActorType>& currentActor = it->second;
		currentActor->OnDraw();
	}
}

void IScene::Init(AppContext* appContext) {
	sceneContext.initailizingFieldActors = false;
	sceneContext.appContext = appContext;
	for (int i = 0; i < sceneContext.deferredActors.size(); i++) {
		DeferredPtr& deferred = sceneContext.deferredActors[i];
		deferred->Make(deferred, &sceneContext);
	}
	sceneContext.deferredActors.clear();
}