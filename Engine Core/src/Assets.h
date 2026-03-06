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
	SDL_Texture* texture;
};

using Asset = std::variant<
	Texture
>;

class AssetManager {
public:
	AssetManager(ResType res, std::type_index id);
	AssetManager() : resource(ResType::Counter), identifierType(typeid(void)) {}
	ResType resource;
private:
	std::type_index identifierType;
	std::vector<Asset> currentTypeAssets;
};

using MetadataType = std::variant<
	std::monostate,
	std::vector<int>,
	std::string,
	int
>;

struct ResourceMetadata {
	std::string filePath, resourceName, fileExtension, fileName;
	std::unordered_map<std::string, MetadataType> properties;
};

class AssetParser {
public:
	AssetParser(AssetManager& manager);
private:
	AssetManager& assetManagerRef;
	bool currentSegmentNumber, isInResourceHeader, parsedHeaderSinceAdded = false, afterPropertyAssignmentChar = false;
	std::string accumilatedString = "", propertyName = "";
	int segmentIndex = 0, specialCharIndex = 0, previousResourceIndex;
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
	void ParseIntegerProperty();
	void ParseVectorProperty();
	void ParseStringProperty();
	std::string GetDefaultExtension();
	std::string GetResourceDirectory();
};