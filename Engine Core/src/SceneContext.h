#pragma once
#include <unordered_map>
#include <memory>

using ActorUUID = uint64_t;

struct AppContext;
struct DeferredActor;
class ActorData;

struct SceneContext {
	AppContext* appContext = nullptr;
	std::unordered_map<ActorUUID, ActorData> sceneActors;
	std::vector<std::unique_ptr<DeferredActor>> deferredActors;
	bool initailizingFieldActors = true;
};