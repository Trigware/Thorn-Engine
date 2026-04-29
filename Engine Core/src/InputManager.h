#pragma once
#include "AppContext.h"
#include "ActionNode.h"

namespace ThornEngine {

class InputManager {
public:
	InputManager(const InputManager& manager) = delete;
	template<Enum T> bool ActionActive(T action, InputType inputType) const {
		KeyExpression& actionKeyExpr = GetKeyExpr(action);
		bool currentActive = actionKeyExpr->Eval(appContext->inputData.currentSet);
		bool previousActive = false;
		if (inputType != InputType::Hold)
			previousActive = actionKeyExpr->Eval(appContext->inputData.prevSet);

		switch (inputType) {
			case InputType::Hold: return currentActive;
			case InputType::Tap: return currentActive && !previousActive;
			case InputType::Release: return !currentActive && previousActive;
		}

		return false;
	}

	template<Enum T> bool IsActionTapped(T action) const { return ActionActive(action, InputType::Tap); }
	template<Enum T> bool IsActionHeld(T action) const { return ActionActive(action, InputType::Hold); }
	template<Enum T> bool IsActionReleased(T action) const { return ActionActive(action, InputType::Release); }
private:
	friend class IScene;
	friend class SceneManager;

	InputManager() = default;
	template<Enum ID> KeyExpression& GetKeyExpr(ID identifier) const {
		return *appContext->template GetAsset<ActionRes, ID>(identifier);
	}

	AppContext* appContext = nullptr;
};

}