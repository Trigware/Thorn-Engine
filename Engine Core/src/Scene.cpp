#include "Scene.h"
#include "AppContext.h"

namespace ThornEngine {

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
	SDL_Event event;
	appContext->inputData.ClearEvents();

	while (SDL_PollEvent(&event)) {
	switch (event.type) {
		case SDL_QUIT: windowRunning = false; break;
		case SDL_KEYDOWN: {
			bool isHeld = event.key.repeat >= 1;
			KeyList& usedList = isHeld ? appContext->inputData.heldKeys : appContext->inputData.tappedKeys;
			SDL_Keycode keyCode = event.key.keysym.sym;
			usedList.push_back(keyCode);
			break;
		}
		case SDL_KEYUP: {
			SDL_Keycode keyCode = event.key.keysym.sym;
			appContext->inputData.releasedKeys.push_back(keyCode);
			break;
		}
	}}
}

void IScene::DrawScene() {
	for (auto it = sceneContext.sceneActors.begin(); it != sceneContext.sceneActors.end(); it++) {
		ActorData& currentActor = it->second;
		currentActor.OnDraw();
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

}