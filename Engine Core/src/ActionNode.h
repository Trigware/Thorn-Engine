#pragma once
#include <string>
#include <memory>
#include "InputData.h"

namespace ThornEngine {

struct IActionNode {
	virtual ~IActionNode() = default;
	virtual std::string AsStr(int nestingLevel) const = 0;
	virtual bool Eval(const InputData& inputData, InputType inputType) const = 0;

	friend std::ostream& operator<<(std::ostream& os, const IActionNode& actionNode) {
		os << actionNode.AsStr(0);
		return os;
	}

	const int indentSpaceCount = 3;
};

using KeyExprNode = std::unique_ptr<IActionNode>;

}