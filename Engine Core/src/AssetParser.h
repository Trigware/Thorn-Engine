#pragma once
#include <unordered_map>
#include <unordered_set>
#include "AssetLexer.h"

namespace ThornEngine {

struct FileAsset {
	std::string fileName, fileExtension;
};

using V4I = std::array<int, 4>;

using AssetSpecificData = std::variant<
	std::monostate,
	FileAsset
>;

using PropertyValue = std::variant<
	int,
	V2I,
	V4I,
	std::string
>;

enum class PropertyKind {
	Unknown,
	TileSize,
	SegmentSizes,
	StrTest
};

struct AssetProperty {
	std::string name = "";
	PropertyKind kind = PropertyKind::Unknown;
	PropertyValue value;
	PropertyType type = PropertyType::NoAssignment;
};

struct AssetData {
	int identifierNumber;
	std::string assetName, tagName;
	AssetSpecificData assetSpecificData;
	HeaderType headerType;
	std::vector<AssetProperty> properties;
};

class AssetParser {
public:
	AssetParser(AssetLexer& lexer);
private:
	static std::unordered_map<std::string, PropertyKind> propertyNameKindMap;
	static std::unordered_map<ResType, std::unordered_set<PropertyKind>> supportedProperties;
	AssetLexer& assetLexerRef;
	AssetData latestAsset;
	AssetProperty latestProperty;
	bool containsHeader = false;
	inline void AddError(ParseErrorType type, std::string message = "") { assetLexerRef.parseErrors.emplace_back(message, type, -1, -1); }
	template<typename T>
	bool ErrorIfInvalidHeader(std::string type) {
		if (std::holds_alternative<std::monostate>(latestAsset.assetSpecificData)) latestAsset.assetSpecificData = T();
		if (!std::holds_alternative<T>(latestAsset.assetSpecificData)) { AddError(ParseErrorType::InvalidHeaderType, "expected " + type); return true; }
		return false;
	}
	template<typename T>
	inline T& SpecificData() { return std::get<T>(latestAsset.assetSpecificData); }
	void ParseVectorValue(std::string vecAsStr);
	void HandleAsset();
	void HandleTexture();
	template<typename TProp, typename T, typename... Other>
	void SetPropertyRecurse(TProp& propertyField, AssetProperty& propertyData) {
		if (std::holds_alternative<T>(propertyData.value)) {
			propertyField = static_cast<TProp>(std::get<T>(propertyData.value));
			return;
		}
		if constexpr (sizeof...(Other) > 0) {
			SetPropertyRecurse<TProp, Other...>(propertyField, propertyData);
			return;
		}
		AddError(ParseErrorType::InvalidPropertyType, propertyData.name);

	}
	template<typename TProp, typename... Other>
	void SetProperty(TProp& propertyField, AssetProperty& propertyData) { SetPropertyRecurse<TProp, TProp, Other...>(propertyField, propertyData); }
	void HandleAssetProperties(Texture& textureAsset);
};

}