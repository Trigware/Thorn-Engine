#pragma once
#include <string>
#include <iostream>
#include <variant>
#include "Assets.h"

namespace ThornEngine {

enum class HeaderType {
	Unknown,
	Path,
	Action
};

enum class IdentifierType {
	Unknown,
	HeaderIdentifier,
	HeaderName,
	HeaderTag,
	HeaderAssign,
	PathFileName,
	PathFileExtension,
	ActionKey,
	ActionOR,
	ActionAND,
	HeaderClose,
	PropertyName,
	PropertyAssign,
	PropertyValue
};

enum class PropertyType {
	NoAssignment,
	Integer,
	Vector,
	String
};

using TokenValue = std::variant<
	std::monostate,
	int,
	std::string,
	PropertyType
>;

enum class SectionType {
	Unknown,
	Number,
	Identifier,
	Symbol
};

enum class CaptureState {
	NotActive,
	HeaderTag,
	PathName,
	PropertyValue,
	ActionKey
};

struct Token {
	IdentifierType identifier = IdentifierType::Unknown;
	TokenValue value;
	Token(IdentifierType id, TokenValue val = std::monostate()) : identifier(id), value(val) {}
	Token() = default;
	std::string TokenAsStr() const;
	friend std::ostream& operator<<(std::ostream& os, const Token& token) {
		os << token.TokenAsStr();
		return os;
	}
	template<typename T>
	inline T GetVal() const { return std::get<T>(value); }
	template<typename T>
	std::string ValToStr() const {
		const T& originValue = GetVal<T>();
		if constexpr (std::is_same_v<T, std::string>) return "\"" + originValue + "\"";
		else if constexpr (std::is_same_v<T, PropertyType>) {
			std::string result = "PropertyType::";
			switch (originValue) {
				case PropertyType::Integer: result += "Integer"; return result;
				case PropertyType::Vector: result += "Vector"; return result;
				case PropertyType::String: result += "String"; return result;
				default: result += "Unknown"; return result;
			}
		}
		else return std::to_string(originValue);
	}
};

struct Section {
	std::string str = "";
	SectionType type = SectionType::Unknown;
	Section() = default;
	Section(std::string txt, SectionType type) : str(txt), type(type) {}
	inline int AsInt() { return std::stoi(str); }
};

enum class ParseErrorType {
	Unknown,
	UnrecognizedHeaderSection,
	UnrecognizedPropertySection,
	InvalidHeaderType,
	FileExtensionRedefinition,
	PropertyAsGlobal,
	UnrecognizedLiteral,
	InvalidIntegerLiteral,
	InvalidVectorLiteral,
	InvalidTerminatingToken,
	IncorrectPropertyPlacement,
	InvalidPropertyKind,
	UnsupportedProperty,
	AssetIdentifierRedefinition,
	InvalidPropertyType,
	IncompatibleHeaderType,
	IncompleteHeaderAssignment,
	UninitializedResource
};

struct ParseError {
	std::string sectionMessage = "", metadataPath = "";
	int rowNumber = -1, columnNumber;
	ParseErrorType errorType = ParseErrorType::Unknown;
	ParseError(const std::string& sectionText, ParseErrorType error, int row = -1, int column = -1, std::string metaPath = "")
		: sectionMessage(sectionText), errorType(error), rowNumber(row), columnNumber(column), metadataPath(metaPath) {}
	friend std::ostream& operator<<(std::ostream& os, const ParseError& error) {
		std::string result = "Metadata Parse Error - ";
		switch (error.errorType) {
			case ParseErrorType::UnrecognizedHeaderSection: result += "unable to deduce a header section"; break;
			case ParseErrorType::UnrecognizedPropertySection: result += "unable to deduce a property section"; break;
			case ParseErrorType::InvalidHeaderType: result += "unable to get header-type-specific value from incompatible type"; break;
			case ParseErrorType::FileExtensionRedefinition: result += "file extension can be defined at most once"; break;
			case ParseErrorType::PropertyAsGlobal: result += "encountered property declared without header"; break;
			case ParseErrorType::UnrecognizedLiteral: result += "attempted to assign literal of unknown type to property"; break;
			case ParseErrorType::InvalidIntegerLiteral: result += "unable to parse an integer literal as a part of vector"; break;
			case ParseErrorType::InvalidVectorLiteral: result += "all vector literals must be exactly 2 or 4 elements long"; break;
			case ParseErrorType::InvalidTerminatingToken: result += "metadata file must end with a full header or a full property"; break;
			case ParseErrorType::IncorrectPropertyPlacement: result += "property name was placed incorrectly"; break;
			case ParseErrorType::InvalidPropertyKind: result += "encountered a property which isn't associated with any behavior"; break;
			case ParseErrorType::UnsupportedProperty: result += "encountered a property which is unsupported on the wanted asset type"; break;
			case ParseErrorType::AssetIdentifierRedefinition: result += "the same asset identifier can be defined once per asset type"; break;
			case ParseErrorType::InvalidPropertyType: result += "wanted to assign a type of value which isn't supported for properties of kind"; break;
			case ParseErrorType::IncompatibleHeaderType: result += "attempted to use a feature from another header type"; break;
			case ParseErrorType::IncompleteHeaderAssignment: result += "expected to encounter non header terminator after header assignment"; break;
			case ParseErrorType::UninitializedResource: result += "encountered an uninitialized resource whose header type enforces assignment"; break;
			default: result += "UNKNOWN ERROR"; break;
		}
		if (!error.sectionMessage.empty()) result += ": " + error.sectionMessage;
		if (error.rowNumber > -1) result += " (row: " + std::to_string(error.rowNumber) + ", column: " + std::to_string(error.columnNumber) + ")";
		if (!error.metadataPath.empty()) result += " (file: " + error.metadataPath + ")";
		os << result;
		return os;
	}
};

class AssetLexer {
public:
	AssetLexer(AssetManager& manager);
	std::vector<Token> tokens;
	std::vector<ParseError> parseErrors;
	AssetManager& assetManagerRef;
	HeaderType headerType = HeaderType::Unknown;
	std::string metadataPath = "";
private:
	std::string metadataContents = "";
	int curIdx = 0, lastNewLineIdx = 0, curLine = 1, curColumn = 0;
	bool inHeader = false, afterHeaderAssign = false, inComment = false, quotedAssetPath = false;
	PropertyType propertyType = PropertyType::NoAssignment;
	CaptureState sectionCaptures = CaptureState::NotActive;
	HeaderType GetHeaderType();
	Section currentSection, prevSection;
	void ParseMetadata();
	SectionType DetermineSectionType(char ch);
	void StartNewSection(SectionType newType);
	inline void AddToken(IdentifierType id, TokenValue val = std::monostate()) { tokens.emplace_back(id, val); }
	void PrintTokens();
	void ParseHeaderSection();
	void ParsePropertySection();
	Token GetPreviousToken();
	bool HandleCaptureState(char ch, SectionType sectionType);
	bool ActivatedComment();
	void PrintLexerErrors();
	bool PropertyPlacedCorrectly();
	inline void ThrowLexerError() { throw std::runtime_error("Encountered an unrecognized section of a metadata file during lexing!"); }
	inline void AddError(ParseErrorType error) { parseErrors.emplace_back(currentSection.str, error, curLine, curColumn, metadataPath); }
	bool CheckIfSettingProperty();
	bool IsInsideString();
	void SetCapture(CaptureState capture, bool resetSection = true) { sectionCaptures = capture; if (resetSection) currentSection.str = ""; }
	inline void CloseHeader() { inHeader = false; AddToken(IdentifierType::HeaderClose); }
	inline CaptureState GetPostAssignCapture() { return headerType == HeaderType::Path ? CaptureState::PathName : CaptureState::ActionKey; }
	void ParseHeaderAssign();
};

}