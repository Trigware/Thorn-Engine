#pragma once
#include <SDL.h>
#include <array>

using KeySet = std::array<Uint8, SDL_NUM_SCANCODES>;

enum class InputType {
	Unknown,
	Tap,
	Hold,
	Release
};

struct InputData {
	KeySet currentSet = {}, prevSet = {};
};