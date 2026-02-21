#include "App.h"
#include <iostream>

App::App(const std::string& initWindowTitle, int initWindowWidth, int initWindowHeight) {
	exitCode = Init(initWindowTitle, initWindowWidth, initWindowHeight);
}

App::~App() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	Mix_CloseAudio();
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
}

ExitCode App::Init(const std::string& title, int width, int height) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) { std::cout << "SDL Initialization ERROR: " << SDL_GetError() << '\n'; return ExitCode::SDLInitError; }
	window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (window == nullptr) { std::cout << "Window Creation ERROR: " << SDL_GetError() << '\n'; return ExitCode::WindowCreationError; }
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) { std::cout << "Render Creation ERROR: " << SDL_GetError() << '\n'; return ExitCode::RendererCreationError; }

	int imageInitFlags = IMG_INIT_PNG;
	if ((IMG_Init(imageInitFlags) & imageInitFlags) == 0) { std::cout << "SDLImage Initialization ERROR: " << SDL_GetError() << '\n'; return ExitCode::SDLImageInitError; }
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) { std::cout << "SDLMixer Initialization ERROR: " << SDL_GetError() << '\n'; return ExitCode::SDLMixerInitError; }
	return ExitCode::Success;
}