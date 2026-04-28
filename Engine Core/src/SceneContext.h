#pragma once
#include <unordered_map>
#include <memory>

namespace ThornEngine {

/// @brief Randomly-generated 64-bit number used to identify actors belonging to a scene tree.
using ActorUUID = uint64_t;

class AppContext;
struct DeferredActor;
class ActorData;

/* @brief Contains scene data (such as actors) and application data (renderer, resources, window).
* 
* Accessable for the user when creating custom components.
* 
* Wrong interference with the SceneContext may lead to unexpected behaviour.
*/
class SceneContext {
public:
	AppContext* appContext = nullptr;
	std::unordered_map<ActorUUID, ActorData> sceneActors;
private:
	friend class Actor;
	friend class IScene;
	std::vector<std::unique_ptr<DeferredActor>> deferredActors;
	bool initailizingFieldActors = true;
};

}