#pragma once
#include "Assets.h"

namespace ThornEngine {

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

}