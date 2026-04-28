#pragma once
#include <iostream>
#include <variant>
#include <vector>
#include <SDL.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <array>
#include <type_traits>
#include <memory>
#include "ResType.h"
#include "Vector.h"
#include "ActionNode.h"

namespace ThornEngine {

template<typename T>
concept Enum = std::is_enum_v<T>;

struct Texture {
	Texture() = default;
	SDL_Texture* texture = nullptr;
	V2I tileSize, upperLeftSegmentSize, bottomRightSegmentSize;
	bool isSegmented = false;
};

class IActionNode;
using KeyExpression = std::unique_ptr<IActionNode>;

using Asset = std::variant<
	Texture,
	KeyExpression
>;

class AppContext;

struct AssetManager {
	AssetManager(ResType res, std::type_index id, AppContext* context);
	AssetManager() : resource(ResType::Counter), identifierType(typeid(void)) {}
	ResType resource;
	std::unordered_map<int, Asset> assets;
	AppContext* appContext = nullptr;
	std::type_index identifierType;
};

template<Resource Res>
using ConditionalAsset =
	std::conditional_t<std::is_same_v<Res, TextureRes>, Texture, KeyExpression
>;

using ManagerPtr = std::unique_ptr<AssetManager>&;

}