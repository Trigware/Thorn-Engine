#pragma once
#include <unordered_map>
#include <memory>

struct IActorType;
struct AppContext;
using ActorUUID = uint64_t;

struct SceneContext {
	AppContext* appContext;
	std::unordered_map<ActorUUID, std::unique_ptr<IActorType>> sceneActors;
};