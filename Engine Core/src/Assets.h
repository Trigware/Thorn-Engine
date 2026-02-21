#pragma once
#include <type_traits>
#include <iostream>

enum class ResType {
	Texture,
	Audio,
	Font,
	Counter
};

using TextureRes = std::integral_constant<ResType, ResType::Texture>;
using AudioRes = std::integral_constant<ResType, ResType::Audio>;
using FontRes = std::integral_constant<ResType, ResType::Font>;

template<typename T>
concept Resource =
	requires{T::value;} &&
	std::is_same_v<T, std::integral_constant<ResType, T::value>> &&
	static_cast<int>(T::value) >= 0 && T::value < ResType::Counter;

template<typename T>
concept Enum = std::is_enum_v<T>;

struct IAssetManager {
	virtual ~IAssetManager() = default;
};

template<ResType Type, Enum ID>
struct AssetManager : public IAssetManager {
};