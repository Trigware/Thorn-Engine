#pragma once
#include <concepts>
#include <typeindex>
#include <string>
#include <unordered_map>
#include "SceneContext.h"

namespace ThornEngine {

class IComponent {
public:
	virtual ~IComponent() = default;
	virtual void OnDraw() {}
protected:
	friend class Actor;
	ActorUUID linkedActorUUID = 0;
};

using ComponentMap = std::unordered_map<std::type_index, std::unique_ptr<IComponent>>;

struct IActorType {
	virtual ~IActorType() = default;
	virtual ComponentMap GetComponents() { return {}; }
	virtual void SetNonRemovableComponents() {}
};

template<typename T> concept ComponentConcept = std::is_base_of_v<IComponent, T> && !std::is_same_v<IComponent, T>;
template<typename T> concept ActorTypeConcept = std::is_base_of_v<IActorType, T> && !std::is_same_v<IActorType, T>;

}