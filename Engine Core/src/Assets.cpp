#include "Assets.h"

AssetManager::AssetManager(ResType res, std::type_index id) : resource(res), identifierType(id) {
	AssetParser parser(*this);
}

AssetParser::AssetParser(AssetManager& manager) : assetManagerRef(manager) {
	std::string metadataPath = GetMetadataPath();
	std::ifstream metaFile(metadataPath);
	std::string currentLine;
	previousResourceIndex = -1;
	while (std::getline(metaFile, currentLine)) ParseLineMetaData(currentLine);
	AddResourceMetadataElement();
}

std::string AssetParser::GetMetadataPath() {
	std::string metadataPath = "Meta/";
	switch (assetManagerRef.resource) {
		case ResType::Texture: metadataPath += "textures"; break;
		case ResType::Audio: metadataPath += "audio"; break;
		case ResType::Font: metadataPath += "fonts"; break;
	}
	return metadataPath + ".meta";
}

void AssetParser::ParseLineMetaData(const std::string& currentLine) {
	accumilatedString = "";
	currentSegmentNumber = isInResourceHeader = afterPropertyAssignmentChar = false;
	segmentIndex = 0;

	bool parsedSinceSegment = false, parsedMeaningfulChar = false;
	for (int i = 0; i < currentLine.size(); i++) {
		char ch = currentLine[i];
		bool isWhitespace = ch == ' ' || ch == '\t';
		if (isWhitespace && (!afterPropertyAssignmentChar || accumilatedString == "")) continue;
		parsedMeaningfulChar = true;
		if (!parsedSinceSegment) currentSegmentNumber = IsNumber(ch);

		bool isCurrentNumber = IsNumber(ch), numberEnding = currentSegmentNumber != isCurrentNumber && currentSegmentNumber, segmentTerminatorChar = !PartOfSegment(ch),
			segmentTerminated = (numberEnding || segmentTerminatorChar) && !afterPropertyAssignmentChar;

		if (segmentTerminated) {
			ParseSegment();
			if (segmentTerminatorChar) ParseSpecialChar(ch);
			else accumilatedString = ch;
			parsedSinceSegment = false;
			continue;
		}

		accumilatedString += ch;
		parsedSinceSegment = true;
	}
	if (!isInResourceHeader && parsedMeaningfulChar) ParsePropertyContent(); 
}

inline bool AssetParser::PartOfSegment(char ch) { return IsNumber(ch) || IsLetter(ch); }
inline bool AssetParser::IsNumber(char ch) { return ch >= '0' && ch <= '9'; }
inline bool AssetParser::IsLetter(char ch) { return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z'; }

void AssetParser::ParseSegment() {
	if (accumilatedString == "") return;
	bool segmentIsNumber = IsNumber(accumilatedString[0]);
	if (segmentIndex == 0) isInResourceHeader = segmentIsNumber;

	if (segmentIsNumber) ParseIntegerSegment();
	else ParseTextualSegment();

	segmentIndex++;
	specialCharIndex = 0;
	accumilatedString = "";
}

void AssetParser::ParseSpecialChar(char specialChar) {
	specialCharIndex++;
	previousSpecialChar = specialChar;
	if (!isInResourceHeader && specialChar == '=') afterPropertyAssignmentChar = true;
}

void AssetParser::ParseIntegerSegment() {
	int numberSegment = std::stoi(accumilatedString);
	bool determingResourceIndex = segmentIndex == 0;
	if (determingResourceIndex && numberSegment - 1 != previousResourceIndex)
		throw std::runtime_error("Each metadata resource must obide my a clear sequential order going from 0!");
	if (!determingResourceIndex) return;
	previousResourceIndex = numberSegment;
	AddResourceMetadataElement();
}

void AssetParser::ParseTextualSegment() {
	if (!isInResourceHeader) { propertyName = accumilatedString; return; }

	parsedHeaderSinceAdded = true;
	switch (segmentIndex) {
		case determiningResourceName:
			currentResource.resourceName = currentResource.fileName = accumilatedString;
			return;
		case lastSegment:
		case middleSegment:
			bool determiningExtension = previousSpecialChar == '.';
			if (determiningExtension) { currentResource.fileExtension = accumilatedString; return; }
			currentResource.fileName = accumilatedString; return;
	}
	throw std::runtime_error("Metadata file has a header which exceeds the number of elements in resource header!");
}

void AssetParser::AddResourceMetadataElement() {
	if (!parsedHeaderSinceAdded) return;
	parsedHeaderSinceAdded = false;
	if (currentResource.fileExtension.empty()) currentResource.fileExtension = GetDefaultExtension();

	currentResource.filePath = GetResourceDirectory() + '/' + currentResource.fileName + '.' + currentResource.fileExtension;
	ResourceMetadata metadata = ResourceMetadata();
}

std::string AssetParser::GetDefaultExtension() {
	switch (assetManagerRef.resource) {
		case ResType::Texture: case ResType::Font: return "png";
		case ResType::Audio: return "wav";
	}
	return "";
}
std::string AssetParser::GetResourceDirectory() {
	switch (assetManagerRef.resource) {
		case ResType::Texture: return "Textures";
		case ResType::Audio: return "Audio";
		case ResType::Font: return "Fonts";
	}
	return "";
}

void AssetParser::ParsePropertyContent() {
	propertyValue = std::monostate();
	char firstSymbol = accumilatedString[0];
	if (firstSymbol == '(') ParseVectorProperty();
	if (firstSymbol == '"') ParseStringProperty();
	if (IsNumber(firstSymbol)) ParseIntegerProperty();

	if (std::holds_alternative<std::monostate>(propertyValue))
		throw std::runtime_error("When parsing a metadata property, the type of it's value could not be determined!");
	if (currentResource.properties.contains(propertyName))
		throw std::runtime_error("Asset metadata contains a redefinition of a property!"); 
	currentResource.properties[propertyName] = propertyValue;
}

void AssetParser::ParseIntegerProperty() {
	propertyValue = 0;
}

void AssetParser::ParseVectorProperty() {
	propertyValue = std::vector<int>{32, 32};
}
void AssetParser::ParseStringProperty() {
	propertyValue = "";
}