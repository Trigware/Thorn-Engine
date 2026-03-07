#pragma once
#include <type_traits>
#include <typeindex>
#include <iostream>
#include <variant>
#include <vector>
#include <SDL.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <array>
#include "Transform.h"

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
	ConditionalAsset<Res> GetAsset(ID identifier) {
		ResType wantedResourceType = static_cast<ResType>(Res::value);
		if (!assetManagers.contains(wantedResourceType))
			throw std::runtime_error("Attempted to obtain an asset of a type which has no associated identification enum!");

		ManagerPtr wantedManager = assetManagers[wantedResourceType];
		if (wantedManager->identifierType != typeid(ID))
			throw std::runtime_error("Attempted to obtain an asset with the incorrect identification enum type!");

		int assetIndex = static_cast<int>(identifier);
		if (!wantedManager->assets.contains(assetIndex))
			throw std::runtime_error("Asset metadata for wanted type isn't synced up with the identification enum type!");

		Asset rawAsset = wantedManager->assets[assetIndex];
		ConditionalAsset<Res> actualAsset = std::get<ConditionalAsset<Res>>(rawAsset);
		return actualAsset;
	}

	std::unordered_map<ResType, std::unique_ptr<AssetManager>> assetManagers;
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
};

using V4I = std::array<int, 4>;

using MetadataType = std::variant<
	std::monostate,
	V2I,
	V4I,
	std::string,
	int
>;

enum class MetadataProperty {
	TileSize,
	SegmentSizes
};

struct ResourceMetadata {
	std::string filePath;
	std::unordered_map<MetadataProperty, MetadataType> properties;
	int resourceIndex = -1;
};

class AssetParser {
public:
	AssetParser(AssetManager& manager);
private:
	AssetManager& assetManagerRef;
	bool currentSegmentNumber, isInResourceHeader, parsedHeaderSinceAdded = false, afterPropertyAssignmentChar = false;
	std::string accumilatedString = "", latestPropertyName = "", latestResourceName, latestExtension, latestFileName;
	int segmentIndex = 0, specialCharIndex = 0;
	ResourceMetadata currentResource;
	MetadataType propertyValue;
	char previousSpecialChar = '\0';
	const static int determiningResourceName = 1, middleSegment = 2, lastSegment = 3;

	std::string GetMetadataPath();
	void ParseLineMetaData(const std::string& currentLine);
	void ParseSegment();
	void ParseSpecialChar(char specialChar);
	void ParseIntegerSegment();
	void ParseTextualSegment();
	inline bool PartOfSegment(char ch);
	inline bool IsNumeral(char ch);
	inline bool IsLetter(char ch);
	inline bool IsSign(char ch);
	int ParseInt(const std::string& numAsStr);
	inline bool IsLastAccumilated(char ch);
	void AddResourceMetadataElement();
	void ParsePropertyContent();
	void ParseVectorProperty();
	void ParseStringProperty();
	void AddNewAsset();
	void AddNewTexture();
	void AssignPropertyToTexture(MetadataProperty currentProperty, const MetadataType& propertyValue, Texture& textureAsset);
	MetadataProperty GetPropertyEnum();
	std::string GetDefaultExtension();
	std::string GetResourceDirectory();
};