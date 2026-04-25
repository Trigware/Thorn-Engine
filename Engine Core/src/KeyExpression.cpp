#include "KeyExpression.h"

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

std::string ActionKey::AsStr(int nestingLevel) const {
	return StrUtils::MakeSpaces(indentSpaceCount * nestingLevel) + "KEY(" + keyAsStr + ")";
}

void KeyExpression::MakeTree() {
	currentIndex = 0;
	rootNode = ParseExpression();
	std::cout << *rootNode << std::endl;
}

KeyExprNode KeyExpression::ParseExpression(int precedenceThreshold) {
	Token nullDenotation = tokens[currentIndex];
	std::string nudStrKey = nullDenotation.GetVal<std::string>();
	KeyExprNode subExpression = ActionKey::Make(nudStrKey);

	for (; currentIndex < tokens.size(); currentIndex++) {
		Token token = tokens[currentIndex];
		bool isOperand = token.identifier == IdentifierType::ActionKey;
		if (isOperand) continue;

		int bindingPower = operatorPrecedence.at(token.identifier);
		if (bindingPower <= precedenceThreshold) break;
		currentIndex++;
		subExpression = ActionOperator::Make(token.identifier, std::move(subExpression), ParseExpression(bindingPower));
	}

	return subExpression;
}

}