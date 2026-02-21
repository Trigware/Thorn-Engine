#pragma once
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <iostream>
#include <type_traits>
#include "Assets.h"

enum class ExitCode {
	Success,
	SDLInitError,
	WindowCreationError,
	RendererCreationError,
	SDLImageInitError,
	SDLMixerInitError
};

class App {
public:
	App(const std::string& initWindowTitle, int initWindowWidth, int initWindowHeight);
	~App();
	inline int GetExitCode() { return static_cast<int>(exitCode); }
	template<typename... Args>
	void CoreResources() {
		static_assert(sizeof...(Args) % 2 == 0, "CoreResources() must receive an even amount of template arguments, because every resource has type and identifier!");
		LoadResourceManager<Args...>();
	}
private:
	ExitCode Init(const std::string& title, int width, int height);
	template<typename Type, typename ID, typename... Rest>
	void LoadResourceManager() {
		std::cout << typeid(Type).name() << " as TYPE, " << typeid(ID).name() << " as ID " << IsResourceValid<Type>() << std::endl;
		if constexpr (sizeof...(Rest) > 0) LoadResourceManager<Rest...>();
	}
	template<typename T>
	constexpr bool IsResourceValid() {
		if (!requires{T::value; }) return false;
		if (!std::is_same_v<std::integral_constant<ResType, T::value>, T>) return false;
		int resourceTypeVal = static_cast<int>(T::value), counterValue = static_cast<int>(ResType::Counter);
		return resourceTypeVal >= 0 && resourceTypeVal < counterValue;
	}
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	ExitCode exitCode = ExitCode::Success;
};