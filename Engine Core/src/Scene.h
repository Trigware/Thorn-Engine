#pragma once
#include <type_traits>
#include "Assets.h"
#include "Actor.h"
#include "InputManager.h"

namespace ThornEngine {

class IScene {
public:
	virtual ~IScene() = default;
	virtual void OnStart() {}
	virtual void OnDraw() {}
	inline Actor Root() { return sceneRoot; }
	template<ActorTypeConcept T, typename... Args> Actor Top(Args&&... args) { return Root().Add<T>(args...); }
	InputManager Input;
private:
	friend class SceneManager;
	void DrawScene();
	void Init(AppContext* appContext);
	SceneContext sceneContext;
	Actor sceneRoot = Actor(&sceneContext);
};

template<typename T>
concept SceneConcept = std::is_base_of_v<IScene, T> && !std::is_same_v<IScene, T>;

class SceneManager {
public:
	SceneManager(AppContext* context) : appContext(context) {};
	template<SceneConcept T>
	void SetStartScene() {
		activeScene = std::make_unique<T>();
		activeScene->Init(appContext);
		activeScene->OnStart();
		ExecuteUpdateLoop();
	}
private:
	void ExecuteUpdateLoop();
	void HandleEvents();
	std::unique_ptr<IScene> activeScene;
	AppContext* appContext = nullptr;
	bool windowRunning = true;
};

}