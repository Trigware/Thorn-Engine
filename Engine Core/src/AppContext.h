#pragma once
#include <unordered_map>
#include <SDL.h>
#include "Assets.h"

struct AppContext {
	std::unordered_map<ResType, std::unique_ptr<IAssetManager>> assetManagers;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
};