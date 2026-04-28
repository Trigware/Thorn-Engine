#pragma once
#include <string>
#include <memory>
#include "InputState.h"
#include "InputData.h"

namespace ThornEngine {

struct IActionNode {
	virtual ~IActionNode() = default;
	virtual std::string AsStr(int nestingLevel) const = 0;
	virtual bool Eval(InputState inputState, const InputData& inputData) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const IActionNode& actionNode) {
		os << actionNode.AsStr(0);
		return os;
	}

	const int indentSpaceCount = 3;
};

using KeyExprNode = std::unique_ptr<IActionNode>;

}