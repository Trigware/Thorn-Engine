#pragma once
#include "Assets.h"
#include "InputData.h"

namespace ThornEngine {

class AppContext {
public:
	template<Resource Res, Enum ID>
	ConditionalAsset<Res>* GetAsset(ID identifier) {
		ResType wantedResourceType = static_cast<ResType>(Res::value);
		if (!assetManagers.contains(wantedResourceType))
			throw std::runtime_error("Attempted to obtain an asset of a type which has no associated identification enum!");

		ManagerPtr wantedManager = assetManagers[wantedResourceType];
		if (wantedManager->identifierType != typeid(ID))
			throw std::runtime_error("Attempted to obtain an asset with the incorrect identification enum type!");

		int assetIndex = static_cast<int>(identifier);
		if (!wantedManager->assets.contains(assetIndex))
			throw std::runtime_error("Asset metadata for wanted type isn't synced up with the identification enum type!");

		Asset& rawAsset = wantedManager->assets[assetIndex];
		ConditionalAsset<Res>& actualAsset = std::get<ConditionalAsset<Res>>(rawAsset);
		return &actualAsset;
	}

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
private:
	friend class App; friend class SceneManager; friend class InputManager;
	std::unordered_map<ResType, std::unique_ptr<AssetManager>> assetManagers;
	InputData inputData;
};

}