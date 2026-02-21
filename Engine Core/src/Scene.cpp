#include "Scene.h"

void SceneManager::ExecuteUpdateLoop() {
	windowRunning = true;
	while (windowRunning) {
		HandleEvents();
		SDL_SetRenderDrawColor(appContext->renderer, 0, 0, 0, 255);
		SDL_RenderClear(appContext->renderer);
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