#pragma once
#include <string>
#include <SDL.h>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <unordered_map>
#include "Assets.h"
#include "Scene.h"
#include "AppContext.h"

namespace ThornEngine {

enum class ExitCode {
	Success,
	SDLInitError,
	WindowCreationError,
	RendererCreationError,
	SDLImageInitError,
	SDLMixerInitError
};

struct AppConfig {
	std::string initWindowTitle;
	int initWidth = 0, initHeight = 0;
};

/* @brief Type representing a Thorn Engine application.
* 
* Must call the CoreResources function in order to link resources to the application.
* Applications which already terminated can be ran with previous configuration and resource options.
*/
class App {
public:
	App(const AppConfig& config) : appConfig(config) { Init(); }
	App(const std::string& title, int width, int height) : appConfig(AppConfig(title, width, height)) { Init(); }
	inline int GetExitCode() { return static_cast<int>(exitCode); }

	template<typename... Args>
	void CoreResources() {
		static_assert(sizeof...(Args) % 2 == 0, "CoreResources() must receive an even amount of template arguments, because every resource has type and identification enum!");
		appContext.assetManagers.clear();
		LoadNewResourceType<Args...>();
	}

	template<SceneConcept T>
	void Run() {
		if (!applicationInitialized) Init();
		SDL_ShowWindow(appContext.window);
		sceneManager.SetStartScene<T>();
		TerminateApplication();
	}

	AppConfig appConfig;
private:
	void Init();

	template<typename Type, typename ID, typename... Rest>
	void LoadNewResourceType() {
		static_assert(Resource<Type>, "Every first type of type pairs in CoreResources() must be valid resource types!");
		static_assert(Enum<ID>, "Every second type of type pairs in CoreResources() must be a user-defined enum!");
		InitializeResourceManager<Type, ID>();
		if constexpr (sizeof...(Rest) > 0) LoadNewResourceType<Rest...>();
	}

	template<Resource Type, Enum ID>
	void InitializeResourceManager() {
		constexpr ResType resourceType = Type::value;
		if (appContext.assetManagers.contains(resourceType)) throw std::runtime_error("Cannot have multiple asset managers handling the same asset type!");
		appContext.assetManagers[resourceType] = std::make_unique<AssetManager>(resourceType, typeid(ID), &appContext);
	}

	void TerminateApplication();

	AppContext appContext;
	SceneManager sceneManager = SceneManager(&appContext);
	ExitCode exitCode = ExitCode::Success;
	bool applicationInitialized = false;
};

}