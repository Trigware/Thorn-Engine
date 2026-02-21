#pragma once
#include <type_traits>

enum class ResType {
	Texture,
	Audio,
	Font,
	Counter
};

using TextureRes = std::integral_constant<ResType, ResType::Texture>;
using AudioRes = std::integral_constant<ResType, ResType::Audio>;
using FontRes = std::integral_constant<ResType, ResType::Font>;