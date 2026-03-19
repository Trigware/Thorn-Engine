#pragma once
#include <iostream>
#include <variant>
#include <vector>
#include <SDL.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <array>
#include "ResType.h"
#include "Transform.h"

namespace ThornEngine {

template<typename T>
concept Enum = std::is_enum_v<T>;

struct Texture {
	Texture() = default;
	SDL_Texture* texture = nullptr;
	V2I tileSize, upperLeftSegmentSize, bottomRightSegmentSize;
	bool isSegmented = false;
};

struct Audio {};
struct Font {};

using Asset = std::variant<
	Texture
>;

struct AppContext;

struct AssetManager {
	AssetManager(ResType res, std::type_index id, AppContext* context);
	AssetManager() : resource(ResType::Counter), identifierType(typeid(void)) {}
	ResType resource;
	std::unordered_map<int, Asset> assets;
	AppContext* appContext = nullptr;
	std::type_index identifierType;
};

struct AppContext {
	template<Resource Res>
	using ConditionalAsset =
		std::conditional_t<std::is_same_v<Res, TextureRes>, Texture,
		std::conditional_t<std::is_same_v<Res, AudioRes>, Audio, Font
	>>;
	using ManagerPtr = std::unique_ptr<AssetManager>&;

	template<Resource Res, Enum ID>
	ConditionalAsset<Res>* GetAsset(ID identifier) {
		ResType wantedResourceType = static_cast<ResType>(Res::value);
		if (!assetManagers.contains(wantedResourceType))
			throw std::runtime_error("Attempted to obtain an asset of a type which has no associated identification enum!");

		ManagerPtr wantedManager = assetManagers[wantedResourceType];
		if (wantedManager->identifierType != typeid(ID))
			throw std::runtime_error("Attempted to obtain an asset with the incorrect identification enum type!");

		int assetIndex = static_cast<int>(identifier);
		if (!wantedManager->assets.contains(assetIndex))
			throw std::runtime_error("Asset metadata for wanted type isn't synced up with the identification enum type!");

		Asset& rawAsset = wantedManager->assets[assetIndex];
		ConditionalAsset<Res>& actualAsset = std::get<ConditionalAsset<Res>>(rawAsset);
		return &actualAsset;
	}

	std::unordered_map<ResType, std::unique_ptr<AssetManager>> assetManagers;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
};

}