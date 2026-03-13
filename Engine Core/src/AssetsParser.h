#pragma once
#include <string>
#include <iostream>
#include <variant>
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

enum class HeaderType {
	Unknown,
	Path
};

enum class IdentifierType {
	Unknown,
	HeaderIdentifier,
	HeaderName,
	HeaderTag,
	HeaderAssign,
	PathFileName,
	PathFileExtension,
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
	HeaderType,
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
	PropertyValue
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
	inline std::string ValToStr() const {
		const T& originValue = std::get<T>(value);
		if constexpr (std::is_same_v<T, std::string>) return "\"" + originValue + "\"";
		else if constexpr (std::is_same_v<T, HeaderType>) {
			std::string result = "HeaderType::";
			switch (originValue) {
				case HeaderType::Path: result += "Path"; return result;
				default: result += "Unknown"; return result;
			}
		}
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

class AssetParser {
public:
	AssetParser(AssetManager& manager);
private:
	static std::unordered_map<std::string, PropertyType> assignmentTypes;
	AssetManager& assetManagerRef;
	std::string GetMetadataPath();
	std::vector<Token> tokens;
	std::string metadataContents = "";
	int curIdx = 0, lastNewLineIdx = 0;
	bool inHeader = false, afterHeaderAssign = false, inComment = false;
	PropertyType propertyType = PropertyType::NoAssignment;
	CaptureState sectionCaptures = CaptureState::NotActive;
	Section currentSection, prevSection;
	void ParseMetadata();
	SectionType DetermineSectionType(char ch);
	inline bool IsNumber(char ch) { return ch >= '0' && ch <= '9'; }
	inline bool IsLetter(char ch) { return ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z'; }
	inline bool IsSymbol(char ch) { return !IsNumber(ch) && !IsLetter(ch); }
	void StartNewSection(SectionType newType);
	inline void AddToken(IdentifierType id, TokenValue val = std::monostate()) { tokens.emplace_back(id, val); }
	void PrintTokens();
	void ParseHeaderSection();
	void ParsePropertySection();
	Token GetPreviousToken();
	bool HandleCaptureState(char ch, SectionType sectionType);
	bool ActivedComment(int index);
	inline void ThrowLexerError() { throw std::runtime_error("Encountered an unrecognized section of a metadata file during lexing!"); }
};

}