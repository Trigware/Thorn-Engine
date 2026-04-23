#pragma once

namespace ThornEngine {

class InputManager {
public:
	InputManager(const InputManager& manager) = delete;
private:
	friend class IScene;
	InputManager() = default;
};

}