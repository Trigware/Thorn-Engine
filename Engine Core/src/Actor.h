#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <typeindex>
#include <variant>
#include "Random.h"
#include "Component.h"
#include "Transform.h"
#include "DrawData.h"

using Component = std::variant<
	Transform*,
	DrawData*
>;

using ActorUUID = uint64_t;
using ComponentMap = std::unordered_map<std::type_index, Component>;

struct IActorType {
	virtual ~IActorType() = default;
	virtual void OnDraw() {}
	std::vector<ActorUUID> subActors;
	ActorUUID superActor = 0;
	bool hasSuperActor = false;
	AppContext* appContext = nullptr;
	virtual inline ComponentMap GetComponents() { return {}; }
};

template<typename T>
concept ActorConcept = std::is_base_of_v<IActorType, T> && !std::is_same_v<IActorType, T>;

struct Container : public IActorType {
};

struct SceneContext {
	AppContext* appContext;
	std::unordered_map<ActorUUID, std::unique_ptr<IActorType>> sceneActors;
};

class Actor {
public:
	Actor(const Actor& copy);
	~Actor();
	constexpr ActorUUID GetUUID() { return selfUUID; }
	using ActorPtr = std::unique_ptr<IActorType>&;
	template<ActorConcept T, typename... Args> Actor Add(Args&&... args) {
		ActorUUID subActorUUID = MakeUUID();
		ActorPtr subActorData = GetActorDataPtr(subActorUUID);
		if constexpr (std::same_as<T, Container>) subActorData = std::make_unique<T>(args...);
		else subActorData = std::make_unique<T>(sceneContext->appContext, args...);

		subActorData->superActor = selfUUID;
		subActorData->hasSuperActor = true;

		ActorPtr selfData = GetActorDataPtr(selfUUID);
		selfData->subActors.push_back(subActorUUID);
		Actor subActorHandle(sceneContext, subActorUUID);
		subActorHandle.deletableOnDestruction = true;
		return subActorHandle;
	}
	template<IsComponent T> T& Get() {
		ActorPtr actorData = GetActorDataPtr(selfUUID);
		ComponentMap components = actorData->GetComponents();
		std::type_index componentType = typeid(T);
		if (!components.contains(componentType))
			throw std::runtime_error("Attempted to get a component which is not available for this actor!");
		Component& genericComponent = components[componentType];
		if (!std::holds_alternative<T*>(genericComponent))
			throw std::runtime_error("Component variant was not updated to include wanted component!");
		T* component = std::get<T*>(genericComponent);
		return *component;
	}
	int GetSubCount() const;
	Actor operator[](int index) const;
	inline bool HasSuper() const { return GetActorDataPtr(selfUUID)->hasSuperActor; }
	Actor Super() const;
	inline void Delete();
	inline bool IsInScene() const { return sceneContext->sceneActors.contains(selfUUID); }
	void DeleteAllSub();
private:
	friend class IScene;
	Actor() {}
	Actor(SceneContext* context);

	ActorUUID selfUUID = 0;
	SceneContext* sceneContext = nullptr;
	bool deletableOnDestruction = false;

	ActorUUID MakeUUID() { return Random<ActorUUID>::Any(); }
	Actor(SceneContext* context, ActorUUID uuid) : sceneContext(context), selfUUID(uuid) {}
	ActorPtr GetActorDataPtr(ActorUUID uuid) const { return sceneContext->sceneActors[uuid]; }
	void DeleteActor(bool manualDeletion);
	void ThrowIfFreed() const;
};