#pragma once
#include "AppContext.h"
#include "ActionNode.h"

namespace ThornEngine {

class InputManager {
public:
	InputManager(const InputManager& manager) = delete;
	template<Enum T> bool ActionActive(T action, InputType inputType) const {
		return GetKeyExpr(action)->Eval(appContext->inputData, inputType);
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