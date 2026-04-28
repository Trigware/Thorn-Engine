#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>
#include "AssetLexer.h"
#include "Utils.h"

namespace ThornEngine {

struct IActionNode {
	virtual ~IActionNode() = default;
	virtual std::string AsStr(int nestingLevel) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const IActionNode& actionNode) {
		os << actionNode.AsStr(0);
		return os;
	}

	const int indentSpaceCount = 3;
};

using KeyExprNode = std::unique_ptr<IActionNode>;
class AssetParser;

enum class SpecialKey {
	Unknown,
	ArrowLeft,
	ArrowRight,
	ArrowUp,
	ArrowDown
};

class ActionKey : public IActionNode {
public:
	static inline KeyExprNode Make(const std::string& keyStr, AssetParser* parserPtr) { return std::make_unique<ActionKey>(keyStr, parserPtr); }
	ActionKey(const std::string& keyStr, AssetParser* parserPtr);

	inline std::string AsStr(int nestingLevel) const override;

private:
	SDL_Keycode GetKeycode();
	SDL_Keycode GetSpecialKey(const std::string& loweredStr);
	void AddError(ParseErrorType errorType, std::string message = "");

	SDL_Keycode key = SDLK_UNKNOWN;
	std::string keyAsStr = "";
	AssetParser* parser = nullptr;
	const inline static std::unordered_map<std::string, SpecialKey> specialKeyMap = {
		{"left", SpecialKey::ArrowLeft}, {"right", SpecialKey::ArrowRight}, {"up", SpecialKey::ArrowUp}, {"down", SpecialKey::ArrowDown}
	};
};

struct ActionOperator : public IActionNode {
	static inline KeyExprNode Make(IdentifierType opType, KeyExprNode&& lhs, KeyExprNode&& rhs) {
		return std::make_unique<ActionOperator>(opType, std::move(lhs), std::move(rhs));
	}

	ActionOperator(IdentifierType opType, KeyExprNode&& lhs, KeyExprNode&& rhs) :
		identifierType(opType), lhsNode(std::move(lhs)), rhsNode(std::move(rhs)) {}

	std::string AsStr(int nestingLevel) const override;

	IdentifierType identifierType = IdentifierType::Unknown;
	KeyExprNode lhsNode, rhsNode;
};

struct KeyExpression {
public:
	inline void AddToken(const Token& identifier) { tokens.push_back(identifier); }
	void MakeTree();
private:
	friend class AssetParser;
	KeyExprNode ParseExpression(int precedenceThreshold = 0);

	const inline static std::unordered_map<IdentifierType, int> operatorPrecedence = { {IdentifierType::ActionOR, 1}, {IdentifierType::ActionAND, 2} };
	std::vector<Token> tokens;
	KeyExprNode rootNode;
	AssetParser* parser = nullptr;
	int currentIndex = 0;
};

}