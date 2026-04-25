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

struct ActionKey : public IActionNode {
	static inline KeyExprNode Make(const std::string& keyStr) { return std::make_unique<ActionKey>(keyStr); }
	ActionKey(const std::string& keyStr) : key(SDL_GetKeyFromName(keyStr.c_str())), keyAsStr(keyStr) {}
	inline std::string AsStr(int nestingLevel) const override;

	SDL_Keycode key = SDLK_UNKNOWN;
	std::string keyAsStr = "";
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
	KeyExprNode ParseExpression(int precedenceThreshold = 0);

	std::unordered_map<IdentifierType, int> operatorPrecedence = { {IdentifierType::ActionOR, 1}, {IdentifierType::ActionAND, 2} };
	std::vector<Token> tokens;
	KeyExprNode rootNode;
	int currentIndex = 0;
};

}