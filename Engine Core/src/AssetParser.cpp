#include <SDL_image.h>
#include "AssetParser.h"
#include "Utils.h"

namespace ThornEngine {

std::unordered_map<std::string, PropertyKind> AssetParser::propertyNameKindMap = {
	{"tileSize", PropertyKind::TileSize},
	{"segmentSizes", PropertyKind::SegmentSizes},
	{"strTest", PropertyKind::StrTest}
};

std::unordered_map<ResType, std::unordered_set<PropertyKind>> AssetParser::supportedProperties = {
	{ResType::Texture, {PropertyKind::TileSize, PropertyKind::SegmentSizes, PropertyKind::StrTest}}
};

AssetParser::AssetParser(AssetLexer& lexer) : assetLexerRef(lexer) {
	Token token;
	for (int i = 0; i < assetLexerRef.tokens.size(); i++) {
		token = assetLexerRef.tokens[i];
		switch (token.identifier) {
			case IdentifierType::HeaderIdentifier:
				if (i > 0) HandleAsset();
				latestAsset.identifierNumber = token.GetVal<int>();
				break;
			case IdentifierType::HeaderName: latestAsset.assetName = token.GetVal<std::string>(); break;
			case IdentifierType::HeaderTag: latestAsset.tagName = token.GetVal<std::string>(); break;
			case IdentifierType::HeaderAssign: latestAsset.headerType = token.GetVal<HeaderType>(); break;
			case IdentifierType::PathFileName:
				if (ErrorIfInvalidHeader<FileAsset>("FileAsset")) break;
				SpecificData<FileAsset>().fileName = token.GetVal<std::string>();
				break;
			case IdentifierType::PathFileExtension: {
				if (ErrorIfInvalidHeader<FileAsset>("FileAsset")) break;
				FileAsset& fileAsset = SpecificData<FileAsset>();
				if (!fileAsset.fileExtension.empty()) AddError(ParseErrorType::FileExtensionRedefinition);
				fileAsset.fileExtension = token.GetVal<std::string>();
				break;
			}
			case IdentifierType::HeaderClose: containsHeader = true; break;
			case IdentifierType::PropertyName:
				latestProperty.name = token.GetVal<std::string>();
				if (!propertyNameKindMap.contains(latestProperty.name)) { AddError(ParseErrorType::InvalidPropertyKind, latestProperty.name); break; }
				latestProperty.kind = propertyNameKindMap[latestProperty.name];
				if (!containsHeader) { AddError(ParseErrorType::PropertyAsGlobal, latestProperty.name); break; }
				break;
			case IdentifierType::PropertyAssign: latestProperty.type = token.GetVal<PropertyType>(); break;
			case IdentifierType::PropertyValue:
				std::string valAsStr = std::get<std::string>(token.value);
				switch (latestProperty.type) {
					case PropertyType::String: latestProperty.value = valAsStr; break;
					case PropertyType::Integer: latestProperty.value = std::stoi(valAsStr); break;
					case PropertyType::Vector: ParseVectorValue(valAsStr); break;
				}
				latestAsset.properties.push_back(latestProperty);
				break;
		}
	}

	if (token.identifier != IdentifierType::HeaderClose && token.identifier != IdentifierType::PropertyValue) {
		AddError(ParseErrorType::InvalidTerminatingToken);
		return;
	}

	HandleAsset();
}

void AssetParser::ParseVectorValue(std::string vecAsStr) {
	std::vector<int> values = {};
	std::string latestNumber = "";
	vecAsStr += ',';
	bool canParseSpace = false, errorIfNumber = false;
	for (int i = 0; i < vecAsStr.size(); i++) {
		char ch = vecAsStr[i];
		if (ch == ',') {
			if (latestNumber.empty()) { AddError(ParseErrorType::InvalidIntegerLiteral); return; }
			values.push_back(std::stoi(latestNumber));
			canParseSpace = true;
			errorIfNumber = false;
			latestNumber = ""; continue;
		}
		if (ch == ' ') {
			if (!canParseSpace) errorIfNumber = true;
			continue;
		}
		if (!StrUtils::IsNumber(ch)) { AddError(ParseErrorType::InvalidIntegerLiteral); return; }
		if (errorIfNumber) { AddError(ParseErrorType::InvalidIntegerLiteral); return; }
		canParseSpace = false;
		latestNumber += ch;
	}

	int numCount = values.size();
	switch (numCount) {
		case 2: latestProperty.value = V2I(values[0], values[1]); return;
		case 4: latestProperty.value = V4I{values[0], values[1], values[2], values[3]}; return;
		default: AddError(ParseErrorType::InvalidVectorLiteral); return;
	}
}

void AssetParser::HandleAsset() {
	AssetManager& assetManager = assetLexerRef.assetManagerRef;
	ResType assetType = assetManager.resource;
	switch (assetType) {
		case ResType::Texture: HandleTexture(); break;
	}
}

void AssetParser::HandleTexture() {
	FileAsset& fileAsset = std::get<FileAsset>(latestAsset.assetSpecificData);
	ResTypeData typeData(ResType::Texture);
	std::string assetPath = typeData.assetDir;
	assetPath += fileAsset.fileName.empty() ? latestAsset.assetName : fileAsset.fileName;
	assetPath += "." + (fileAsset.fileExtension.empty() ? typeData.defaultExtension : fileAsset.fileExtension);

	Texture textureAsset;
	AssetManager& assetManager = assetLexerRef.assetManagerRef;
	textureAsset.texture = IMG_LoadTexture(assetManager.appContext->renderer, assetPath.c_str());
	HandleAssetProperties(textureAsset);
	assetManager.assets[latestAsset.identifierNumber] = textureAsset;
}

void AssetParser::HandleAssetProperties(Texture& textureAsset) {
	AssetManager& assetManager = assetLexerRef.assetManagerRef;
	int propertyCount = latestAsset.properties.size();
	for (int i = 0; i < propertyCount; i++) {
		AssetProperty& property = latestAsset.properties[i];
		bool propertySupported = supportedProperties[assetManager.resource].contains(property.kind);
		if (property.kind == PropertyKind::Unknown) continue;
		if (!propertySupported) { AddError(ParseErrorType::InvalidPropertyType, property.name); continue; }

		switch (property.kind) {
			case PropertyKind::TileSize: SetProperty<V2I, int>(textureAsset.tileSize, property); break;
			case PropertyKind::SegmentSizes:
				V4I segmentSizes; SetProperty(segmentSizes, property);
				textureAsset.upperLeftSegmentSize = V2I(segmentSizes[0], segmentSizes[1]);
				textureAsset.bottomRightSegmentSize = V2I(segmentSizes[2], segmentSizes[3]); break;
			case PropertyKind::StrTest:
				std::string test;
				SetProperty(test, property);
				std::cout << test << std::endl;
				break;
		}
	}

	int idNumber = latestAsset.identifierNumber;
	if (assetManager.assets.contains(idNumber)) { AddError(ParseErrorType::AssetIdentifierRedefinition, std::to_string(idNumber)); return; }
	latestAsset = AssetData();
}

}