#include "KeyExpression.h"
#include "AssetParser.h"

namespace ThornEngine {

std::string ActionOperator::AsStr(int nestingLevel) const {
	std::string result = StrUtils::MakeSpaces(indentSpaceCount * nestingLevel) + "OP(";
	switch (identifierType) {
		case IdentifierType::ActionAND: result += "AND"; break;
		case IdentifierType::ActionOR: result += "OR"; break;
	}
	result += ")\n";

	result += lhsNode->AsStr(nestingLevel + 1) + '\n';
	result += rhsNode->AsStr(nestingLevel + 1);
	return result;
}

SDL_Keycode ActionKey::GetKeycode() {
	std::string loweredStr = StrUtils::ToLower(keyAsStr);
	int isNotChar = loweredStr.size() != 1;
	if (isNotChar) return GetSpecialKey(loweredStr);

	char keyChar = loweredStr[0];
	bool isAlphaNum = StrUtils::IsAlphaNum(keyChar);
	if (!isAlphaNum) return SDLK_UNKNOWN;
	return (SDL_Keycode)keyChar;
}

SDL_Keycode ActionKey::GetSpecialKey(const std::string& loweredStr) {
	bool isValid = specialKeyMap.contains(loweredStr);
	if (!isValid) return SDLK_UNKNOWN;
	SpecialKey specialKey = ActionKey::specialKeyMap.at(loweredStr);

	switch (specialKey) {
		case SpecialKey::ArrowLeft: return SDLK_LEFT;
		case SpecialKey::ArrowRight: return SDLK_RIGHT;
		case SpecialKey::ArrowUp: return SDLK_UP;
		case SpecialKey::ArrowDown: return SDLK_DOWN;
		default: return SDLK_UNKNOWN;
	}
}

std::string ActionKey::AsStr(int nestingLevel) const {
	return StrUtils::MakeSpaces(indentSpaceCount * nestingLevel) + "KEY(" + keyAsStr + ")";
}

ActionKey::ActionKey(const std::string& keyStr, AssetParser* parserPtr) {
	keyAsStr = keyStr;
	parser = parserPtr;
	key = GetKeycode();
	bool isKeyInvalid = key == SDLK_UNKNOWN;
	if (isKeyInvalid) AddError(ParseErrorType::UnrecognizedAction, keyStr);
}

void KeyExpression::MakeTree() {
	currentIndex = -1;
	rootNode = ParseExpression(0);
}

void ActionKey::AddError(ParseErrorType errorType, std::string message) { parser->AddError(errorType, message); }

KeyExprNode KeyExpression::ParseExpression(int precedenceThreshold) {
	currentIndex++;
	Token nullDelimiterToken = tokens[currentIndex];
	std::string nullDelimiterKey = nullDelimiterToken.GetStrVal();
	KeyExprNode subExpr = ActionKey::Make(nullDelimiterKey, parser);

	while (true) {
		currentIndex++;
		bool encounteredEOF = currentIndex >= tokens.size();
		if (encounteredEOF) break;

		Token currentOperator = tokens[currentIndex];
		int bindingPower = KeyExpression::operatorPrecedence.at(currentOperator.identifier);

		bool endSubExpr = bindingPower < precedenceThreshold;
		if (endSubExpr) { currentIndex--; break; }

		subExpr = ActionOperator::Make(currentOperator.identifier, std::move(subExpr), ParseExpression(bindingPower));
	}

	return subExpr;
}

}