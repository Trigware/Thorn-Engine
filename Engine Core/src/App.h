#pragma once
#include <string>
#include <SDL.h>
#include <memory>
#include <stdexcept>
#include "Assets.h"
#include "AppContext.h"
#include "Scene.h"

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
	int initWidth, initHeight;
	AppConfig(const std::string& title, int width, int height) : initWindowTitle(title), initWidth(width), initHeight(height) {}
};

class App {
public:
	App(const AppConfig& config);
	inline int GetExitCode() { return static_cast<int>(exitCode); }

	template<typename... Args>
	void CoreResources() {
		static_assert(sizeof...(Args) % 2 == 0, "CoreResources() must receive an even amount of template arguments, because every resource has type and identification enum!");
		appContext.assetManagers.clear();
		LoadNewResourceType<Args...>();
	}

	template<Scene T>
	void Run() {
		if (hasTerminated) Init();
		sceneManager.SetStartScene<T>();
		TerminateApplication();
		hasTerminated = true;
	}

	AppConfig appConfig;
private:
	ExitCode Init();

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
		appContext.assetManagers[resourceType] = std::make_unique<AssetManager<resourceType, ID>>();
	}

	void TerminateApplication();

	AppContext appContext;
	SceneManager sceneManager = SceneManager(&appContext);
	ExitCode exitCode = ExitCode::Success;
	bool hasTerminated = false;
};