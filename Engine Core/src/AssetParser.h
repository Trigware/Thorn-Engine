#pragma once
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

struct AssetProperty {
	std::string name;
	PropertyValue value;
	PropertyType type;
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
};

}