#include "App.h"
#include <iostream>
#include <SDL_image.h>
#include <SDL_mixer.h>

namespace ThornEngine {

void App::TerminateApplication() {
	SDL_DestroyWindow(appContext.window);
	SDL_DestroyRenderer(appContext.renderer);
	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	applicationInitialized = false;
}

void App::Init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		std::cout << "SDL Initialization ERROR: " << SDL_GetError() << '\n';
		exitCode = ExitCode::SDLInitError;
		return;
	}

	appContext.window = SDL_CreateWindow(appConfig.initWindowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		appConfig.initWidth, appConfig.initHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
	if (appContext.window == nullptr) {
		std::cout << "Window Creation ERROR: " << SDL_GetError() << '\n';
		exitCode = ExitCode::WindowCreationError;
		return;
	}

	appContext.renderer = SDL_CreateRenderer(appContext.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (appContext.renderer == nullptr) {
		std::cout << "Render Creation ERROR: " << SDL_GetError() << '\n';
		exitCode = ExitCode::RendererCreationError;
		return;
	}

	int imageInitFlags = IMG_INIT_PNG;
	if ((IMG_Init(imageInitFlags) & imageInitFlags) == 0) {
		std::cout << "SDLImage Initialization ERROR: " << SDL_GetError() << '\n';
		exitCode = ExitCode::SDLImageInitError;
		return;
	}

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		std::cout << "SDLMixer Initialization ERROR: " << SDL_GetError() << '\n';
		exitCode = ExitCode::SDLMixerInitError;
		return;
	}

	exitCode = ExitCode::Success;
	applicationInitialized = true;
	return;
}

}