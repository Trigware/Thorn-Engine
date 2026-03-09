#pragma once
#include <unordered_map>
#include <memory>

using ActorUUID = uint64_t;

struct IActorType;

template<typename T>
concept ActorConcept = std::is_base_of_v<IActorType, T> && !std::is_same_v<IActorType, T>;

struct AppContext;
struct DeferredActor;
class IScene;

struct SceneContext {
	AppContext* appContext = nullptr;
	std::unordered_map<ActorUUID, std::unique_ptr<IActorType>> sceneActors;
	std::vector<std::unique_ptr<DeferredActor>> deferredActors;
	bool initailizingFieldActors = true;
};