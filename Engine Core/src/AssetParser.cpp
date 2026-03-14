#include "AssetParser.h"

namespace ThornEngine {

AssetParser::AssetParser(AssetLexer& lexer) : assetLexerRef(lexer) {
	for (int i = 0; i < assetLexerRef.tokens.size(); i++) {
		Token token = assetLexerRef.tokens[i];
		switch (token.identifier) {
			case IdentifierType::HeaderIdentifier: latestAsset.identifierNumber = token.GetVal<int>(); break;
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
				if (!containsHeader) { AddError(ParseErrorType::PropertyAsGlobal, latestProperty.name); break; }
				break;
			case IdentifierType::PropertyAssign: latestProperty.type = token.GetVal<PropertyType>(); break;
			case IdentifierType::PropertyValue:
				std::string valAsStr = std::get<std::string>(token.value);
				std::cout << valAsStr << std::endl;
				switch (latestProperty.type) {
					case PropertyType::String: latestProperty.value = valAsStr; break;
				}
				break;
		}
	}
}

}