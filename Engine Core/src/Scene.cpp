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
	KeySet& currentSet = appContext->inputData.currentSet;
	appContext->inputData.prevSet = currentSet;
	int numKeyCount = 0;
	const Uint8* keySetPtr = SDL_GetKeyboardState(&numKeyCount);
	std::copy(keySetPtr, keySetPtr + SDL_NUM_SCANCODES, currentSet.begin());

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
	switch (event.type) {
		case SDL_QUIT: windowRunning = false; break;
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