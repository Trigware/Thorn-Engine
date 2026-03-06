#pragma once
#include <unordered_map>
#include <SDL.h>
#include "Assets.h"

struct AppContext {
	std::unordered_map<ResType, AssetManager> assetManagers;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
};