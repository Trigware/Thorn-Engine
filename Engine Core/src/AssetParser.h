#pragma once
#include <unordered_map>
#include <unordered_set>
#include "AssetLexer.h"
#include "KeyExpression.h"

namespace ThornEngine {

struct FileAsset {
	std::string fileName, fileExtension;
};

using V4I = std::array<int, 4>;

using AssetSpecificData = std::variant<
	std::monostate,
	FileAsset,
	KeyExpressionBuilder
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
	SegmentSizes
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
	std::vector<AssetProperty> properties;
	bool idNumChanged = false;
};

class AssetParser {
public:
	AssetParser(AssetLexer& lexer);
	inline void AddError(ParseErrorType type, std::string message = "") { assetLexerRef.parseErrors.emplace_back(message, type, -1, -1, assetLexerRef.metadataPath); }
private:
	static std::unordered_map<std::string, PropertyKind> propertyNameKindMap;
	static std::unordered_map<ResType, std::unordered_set<PropertyKind>> supportedProperties;
	AssetLexer& assetLexerRef;
	AssetData latestAsset;
	AssetProperty latestProperty;
	HeaderType headerType = HeaderType::Unknown;
	bool containsHeader = false;
	int index = 0;
	template<typename T>
	bool ErrorIfInvalidHeader(std::string type = "") {
		if (std::holds_alternative<std::monostate>(latestAsset.assetSpecificData)) latestAsset.assetSpecificData = T();
		if (!std::holds_alternative<T>(latestAsset.assetSpecificData)) { AddError(ParseErrorType::InvalidHeaderType, "expected " + type); return true; }
		return false;
	}
	template<typename T>
	inline T& SpecificData() { return std::get<T>(latestAsset.assetSpecificData); }
	void ParseVectorValue(std::string vecAsStr);
	void HandleAsset();
	void HandleTexture();
	void HandleAction();
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
	inline Token GetToken(int index) { return assetLexerRef.tokens[index]; }
	inline bool WasPrevToken(IdentifierType identifier) { return index > 0 && GetToken(index - 1).identifier == identifier; }
	inline bool UninitAllowed() {
		return assetLexerRef.headerType == HeaderType::Path;
	}
};

}