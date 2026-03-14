#include "Assets.h"
#include "AssetLexer.h"
#include "SDL_image.h"
#include "AssetParser.h"

namespace ThornEngine {

AssetManager::AssetManager(ResType res, std::type_index id, AppContext* context) : resource(res), identifierType(id), appContext(context) {
	AssetLexer lexer(*this);
}

std::unordered_map<std::string, PropertyType> AssetLexer::assignmentTypes = {
	{"=", PropertyType::Integer},
	{"=\"", PropertyType::String},
	{"=(", PropertyType::Vector}
};

std::string AssetLexer::GetMetadataPath() {
	std::string metadataPath = "Meta/";
	switch (assetManagerRef.resource) {
		case ResType::Texture: metadataPath += "textures"; break;
		case ResType::Audio: metadataPath += "audio"; break;
		case ResType::Font: metadataPath += "fonts"; break;
	}
	return metadataPath + ".meta";
}

AssetLexer::AssetLexer(AssetManager& manager) : assetManagerRef(manager) {
	std::string metadataPath = GetMetadataPath(), currentLine = "";
	std::ifstream metaFile(metadataPath);
	while (std::getline(metaFile, currentLine)) metadataContents += currentLine + '\n';
	ParseMetadata();
	PrintTokens();

	AssetParser parser(*this);
	PrintLexerErrors();
	if (parseErrors.size() == 0) return;
	ThrowLexerError();
}

void AssetLexer::PrintTokens() {
	for (int i = 0; i < tokens.size(); i++) {
		Token token = tokens[i];
		std::cout << token << std::endl;
	}
}

void AssetLexer::PrintLexerErrors() {
	for (int i = 0; i < parseErrors.size(); i++) {
		ParseError error = parseErrors[i];
		std::cout << error << std::endl;
	}
	if (parseErrors.size() > 0) std::cout << "\n";
}

Token AssetLexer::GetPreviousToken() {
	int tokenCount = tokens.size(), index = tokenCount - 1;
	if (index < 0 || index >= tokenCount) return Token();
	return tokens[index];
}

std::string Token::TokenAsStr() const {
	std::string typeAsStr = "Unknown", valueAsStr;
	switch (identifier) {
		case IdentifierType::HeaderIdentifier: typeAsStr = "HeaderIdentifier"; valueAsStr = ValToStr<int>(); break;
		case IdentifierType::HeaderName: typeAsStr = "HeaderName"; valueAsStr = ValToStr<std::string>(); break;
		case IdentifierType::HeaderTag: typeAsStr = "HeaderTag"; valueAsStr = ValToStr<std::string>(); break;
		case IdentifierType::HeaderAssign: typeAsStr = "HeaderAssign"; valueAsStr = ValToStr<HeaderType>(); break;
		case IdentifierType::PathFileName: typeAsStr = "PathFileName"; valueAsStr = ValToStr<std::string>(); break;
		case IdentifierType::PathFileExtension: typeAsStr = "PathFileExtension"; valueAsStr = ValToStr<std::string>(); break;
		case IdentifierType::HeaderClose: typeAsStr = "HeaderClose"; break;
		case IdentifierType::PropertyName: typeAsStr = "PropertyName"; valueAsStr = ValToStr<std::string>(); break;
		case IdentifierType::PropertyAssign: typeAsStr = "PropertyAssign"; valueAsStr = ValToStr<PropertyType>(); break;
		case IdentifierType::PropertyValue: typeAsStr = "PropertyValue"; valueAsStr = ValToStr<std::string>(); break;
	}

	std::string result = typeAsStr + "(";
	if (!valueAsStr.empty()) result += valueAsStr;
	result += ")";
	return result;
}

bool AssetLexer::HandleCaptureState(char ch, SectionType sectionType) {
	std::string newSectionStr = "";
	switch (sectionCaptures) {
		case CaptureState::NotActive: return false;
		case CaptureState::HeaderTag:
			if (ch != ')') return false;
			AddToken(IdentifierType::HeaderTag, currentSection.str);
			break;
		case CaptureState::PathName:
			if (ch != '.') return false;
			AddToken(IdentifierType::PathFileName, currentSection.str);
			newSectionStr = ".";
			break;
		case CaptureState::PropertyValue:
			bool closingVec = propertyType == PropertyType::Vector && ch == ')', closingStr = propertyType == PropertyType::String && ch == '"';
			if (!closingVec && !closingStr) return false;
			AddToken(IdentifierType::PropertyValue, currentSection.str);
			propertyType = PropertyType::NoAssignment;
			break;
	}

	sectionCaptures = CaptureState::NotActive;
	currentSection.str = newSectionStr;
	currentSection.type = sectionType;
	return true;
}

void AssetLexer::ParseMetadata() {
	for (curIdx = 0; curIdx < metadataContents.size(); curIdx++) {
		char ch = metadataContents[curIdx];
		if (ch == '\n') { lastNewLineIdx = curIdx; inComment = false; curLine++; curColumn = 0; }
		curColumn++;
		if (inComment) continue;

		SectionType charType = DetermineSectionType(ch);
		if (HandleCaptureState(ch, charType)) continue;
		bool capturesNonActive = sectionCaptures == CaptureState::NotActive;
		bool newSectionByWhitespace = ch == '\n' || ch == ' ' && currentSection.type != SectionType::Symbol && capturesNonActive;
		if (newSectionByWhitespace) { StartNewSection(SectionType::Unknown); continue; }

		if (currentSection.str.empty()) currentSection.type = charType;
		bool newSectionByType = charType != currentSection.type && capturesNonActive, activatedCommentNow = ActivatedComment();
		if (activatedCommentNow) {
			inComment = true;
			continue;
		}
		if (newSectionByType) StartNewSection(charType);
		bool appendingToSection = ch != ' ' || !capturesNonActive;
		if (appendingToSection) currentSection.str += ch;
	}
}

bool AssetLexer::ActivatedComment() {
	if (propertyType == PropertyType::String) return false;
	int metadataSize = metadataContents.size();
	if (curIdx + 1 >= metadataSize) return false;
	char ch = metadataContents[curIdx], nextCh = metadataContents[curIdx + 1];
	return ch == '/' && nextCh == '/';
}

SectionType AssetLexer::DetermineSectionType(char ch) {
	if (IsLetter(ch)) return SectionType::Identifier;
	if (IsSymbol(ch)) return SectionType::Symbol;
	return currentSection.type == SectionType::Identifier ? SectionType::Identifier : SectionType::Number;
}

void AssetLexer::StartNewSection(SectionType newType) {
	if (currentSection.str.empty()) return;
	bool startedHeader = prevSection.type == SectionType::Number && currentSection.str == ".[";
	if (!startedHeader && !inHeader) ParsePropertySection();

	if (inHeader) ParseHeaderSection();
	if (startedHeader) { inHeader = true; AddToken(IdentifierType::HeaderIdentifier, prevSection.AsInt()); afterHeaderAssign = false; }

	prevSection = currentSection;
	currentSection.str = "";
	currentSection.type = newType;
}

void AssetLexer::ParseHeaderSection() {
	Token prevToken = GetPreviousToken();
	if (prevToken.identifier == IdentifierType::HeaderIdentifier) { AddToken(IdentifierType::HeaderName, currentSection.str); return; }
	bool isInHeaderTag = (prevToken.identifier == IdentifierType::HeaderName || prevToken.identifier == IdentifierType::PathFileExtension)
		&& currentSection.str == "(";
	if (isInHeaderTag) { sectionCaptures = CaptureState::HeaderTag; return; }

	if (!afterHeaderAssign && currentSection.str == "=") {
		AddToken(IdentifierType::HeaderAssign, HeaderType::Path);
		sectionCaptures = CaptureState::PathName;
		afterHeaderAssign = true;
		return;
	}

	bool beforeExtension = prevToken.identifier == IdentifierType::PathFileName || prevToken.identifier == IdentifierType::HeaderName || prevToken.identifier == IdentifierType::HeaderTag;
	if (prevSection.str == "." && beforeExtension) {
		AddToken(IdentifierType::PathFileExtension, currentSection.str);
		return;
	}

	if (currentSection.str == "]") {
		inHeader = false;
		AddToken(IdentifierType::HeaderClose);
		return;
	}

	if (currentSection.str == "." && beforeExtension) return;
	AddError(ParseErrorType::UnrecognizedHeaderSection);
}

void AssetLexer::ParsePropertySection() {
	Token prevToken = GetPreviousToken();
	if (currentSection.type == SectionType::Identifier) {
		if (prevToken.identifier == IdentifierType::PropertyAssign) { AddError(ParseErrorType::UnrecognizedLiteral); return; }
		AddToken(IdentifierType::PropertyName, currentSection.str);
		return;
	}

	if (assignmentTypes.contains(currentSection.str)) {
		propertyType = assignmentTypes[currentSection.str];
		if (propertyType != PropertyType::Integer) sectionCaptures = CaptureState::PropertyValue;
		AddToken(IdentifierType::PropertyAssign, propertyType);
		return;
	}

	if (propertyType == PropertyType::Integer) {
		AddToken(IdentifierType::PropertyValue, currentSection.str);
		propertyType = PropertyType::NoAssignment;
		return;
	}

	if (currentSection.str == "\t" && curIdx - lastNewLineIdx == 2) return;
	if (currentSection.type == SectionType::Number && propertyType == PropertyType::NoAssignment) return;
	if (currentSection.str == ";" && prevToken.identifier == IdentifierType::PropertyValue) return;
	AddError(ParseErrorType::UnrecognizedPropertySection);
}

}