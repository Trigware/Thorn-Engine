#include "Assets.h"
#include "SDL_image.h"

AssetManager::AssetManager(ResType res, std::type_index id, AppContext* context) : resource(res), identifierType(id), appContext(context) {
	AssetParser parser(*this);
}

AssetParser::AssetParser(AssetManager& manager) : assetManagerRef(manager) {
	std::string metadataPath = GetMetadataPath();
	std::ifstream metaFile(metadataPath);
	std::string currentLine;
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
		if (!parsedSinceSegment) currentSegmentNumber = IsNumeral(ch);

		bool isCurrentNumber = IsNumeral(ch), numberEnding = currentSegmentNumber != isCurrentNumber && currentSegmentNumber, segmentTerminatorChar = !PartOfSegment(ch),
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

inline bool AssetParser::PartOfSegment(char ch) { return IsNumeral(ch) || IsLetter(ch); }
inline bool AssetParser::IsNumeral(char ch) { return ch >= '0' && ch <= '9'; }
inline bool AssetParser::IsLetter(char ch) { return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z'; }
inline bool AssetParser::IsSign(char ch) { return ch == '+' || ch == '-'; }
inline bool AssetParser::IsLastAccumilated(char ch) { return accumilatedString.size() > 0 && accumilatedString[accumilatedString.size() - 1] == ch; }

void AssetParser::ParseSegment() {
	if (accumilatedString == "") return;
	bool segmentIsNumber = IsNumeral(accumilatedString[0]);
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
	if (!determingResourceIndex) return;
	AddResourceMetadataElement();
	currentResource.resourceIndex = numberSegment;
}

void AssetParser::ParseTextualSegment() {
	if (!isInResourceHeader) { latestPropertyName = accumilatedString; return; }

	parsedHeaderSinceAdded = true;
	switch (segmentIndex) {
		case determiningResourceName:
			latestResourceName = latestFileName = accumilatedString;
			return;
		case lastSegment:
		case middleSegment:
			bool determiningExtension = previousSpecialChar == '.';
			if (determiningExtension) { latestExtension = accumilatedString; return; }
			latestFileName = accumilatedString; return;
	}
	throw std::runtime_error("Metadata file has a header which exceeds the number of elements in resource header!");
}

void AssetParser::AddResourceMetadataElement() {
	if (!parsedHeaderSinceAdded) return;
	parsedHeaderSinceAdded = false;
	if (latestExtension.empty()) latestExtension = GetDefaultExtension();

	currentResource.filePath = GetResourceDirectory() + '/' + latestFileName + '.' + latestExtension;
	AddNewAsset();
	currentResource = ResourceMetadata();
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
	char firstSymbol = accumilatedString.size() == 0 ? '\0' : accumilatedString[0];
	if (firstSymbol == '(') ParseVectorProperty();
	if (firstSymbol == '"') ParseStringProperty();
	if (IsNumeral(firstSymbol) || IsSign(firstSymbol)) propertyValue = ParseInt(accumilatedString);

	if (std::holds_alternative<std::monostate>(propertyValue))
		throw std::runtime_error("When parsing a metadata property, the type of it's value could not be determined!");

	MetadataProperty currentProperty = GetPropertyEnum();
	if (currentResource.properties.contains(currentProperty))
		throw std::runtime_error("Asset metadata contains a redefinition of a property!"); 
	currentResource.properties[currentProperty] = propertyValue;
}

MetadataProperty AssetParser::GetPropertyEnum() {
	if (latestPropertyName == "tileSize") return MetadataProperty::TileSize;
	if (latestPropertyName == "segmentSizes") return MetadataProperty::SegmentSizes;
	throw std::runtime_error("Encountered an unknown asset metadata property!");
}

int AssetParser::ParseInt(const std::string& numAsStr) {
	for (int i = 0; i < numAsStr.size(); i++) {
		char ch = numAsStr[i];
		if (!IsNumeral(ch) && (!IsSign(ch) || i > 0)) throw std::runtime_error("Encountered an invalid character while trying to parse an integer in a metadata file!");
	}
	int result = std::stoi(numAsStr);
	return result;
}

void AssetParser::ParseVectorProperty() {
	int valueSize = accumilatedString.size();
	if (!IsLastAccumilated(')')) throw std::runtime_error("Expected a ')' when ending a vector literal!");
	std::vector<int> result;
	std::string possibleNumberAsStr = "";

	for (int i = 1; i < valueSize-1; i++) {
		char ch = accumilatedString[i];
		if (ch == ' ') continue;
		if (ch == ',') {
			result.push_back(ParseInt(possibleNumberAsStr));
			possibleNumberAsStr = "";
			continue;
		}
		possibleNumberAsStr += ch;
	}
	if (possibleNumberAsStr != "") result.push_back(ParseInt(possibleNumberAsStr));

	switch (result.size()) {
		case 2: propertyValue = V2I(result[0], result[1]); return;
		case 4: propertyValue = V4I{result[0], result[1], result[2], result[3]}; return;
	}
	throw std::runtime_error("Metadata property value vectors can only have 2 or 4 integer elements!");
}

void AssetParser::ParseStringProperty() {
	if (!IsLastAccumilated('"')) throw std::runtime_error("Expected a '\"' when ending a string literal!");
	std::string result = accumilatedString.substr(1, accumilatedString.size() - 2);
	propertyValue = result;
}

void AssetParser::AddNewAsset() {
	switch (assetManagerRef.resource) {
		case ResType::Texture: AddNewTexture(); break;
	}
}

void AssetParser::AddNewTexture() {
	if (assetManagerRef.assets.contains(currentResource.resourceIndex))
		throw std::runtime_error("Metadata file contains a redefinition of an identifier index!");

	Texture textureAsset;
	SDL_Surface* textureSurface = IMG_Load(currentResource.filePath.c_str());
	textureAsset.tileSize = V2I(textureSurface->w, textureSurface->h);
	if (textureSurface == nullptr) throw std::runtime_error("The asset path specified in metadata is either invalid or can't be loaded!");

	SDL_Texture* texture = SDL_CreateTextureFromSurface(assetManagerRef.appContext->renderer, textureSurface);
	if (texture == nullptr) throw std::runtime_error("An error occured while constructing a texture!");
	SDL_FreeSurface(textureSurface);

	textureAsset.texture = texture;
	for (auto it = currentResource.properties.begin(); it != currentResource.properties.end(); it++) {
		MetadataProperty currentProperty = it->first;
		MetadataType propertyValue = it->second;
		AssignPropertyToTexture(currentProperty, propertyValue, textureAsset);
	}
	assetManagerRef.assets[currentResource.resourceIndex] = textureAsset;
}

void AssetParser::AssignPropertyToTexture(MetadataProperty currentProperty, const MetadataType& propertyValue, Texture& textureAsset) {
	switch (currentProperty) {
	case MetadataProperty::TileSize:
		if (std::holds_alternative<V2I>(propertyValue)) { textureAsset.tileSize = std::get<V2I>(propertyValue); return; }
		if (std::holds_alternative<int>(propertyValue)) { textureAsset.tileSize = V2I(std::get<int>(propertyValue)); return; }
		throw std::runtime_error("The tileSize metadata property can be only either int or V2I!");
	case MetadataProperty::SegmentSizes:
		if (std::holds_alternative<V4I>(propertyValue)) {
			V4I actualValue = std::get<V4I>(propertyValue);
			textureAsset.upperLeftSegmentSize = V2I(actualValue[0], actualValue[1]);
			textureAsset.bottomRightSegmentSize = V2I(actualValue[2], actualValue[3]);
			textureAsset.isSegmented = true;
			return;
		}
		throw std::runtime_error("The segmentSizes metadata property can only be a V4I!");
	}
	throw std::runtime_error("Texture asset only supports the 'tileSize' and 'segmentSizes' properties!");
}