#pragma once
#include <vector>
#include <SDL.h>

using KeyList = std::vector<SDL_Keycode>;

struct InputData {
	void ClearEvents() { tappedKeys.clear(); heldKeys.clear(); releasedKeys.clear(); }
	KeyList tappedKeys, heldKeys, releasedKeys;
};