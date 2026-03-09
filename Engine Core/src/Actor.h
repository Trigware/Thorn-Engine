#pragma once
#include <iostream>
#include <memory>
#include <typeindex>
#include <variant>
#include <utility>
#include <functional>
#include "Random.h"
#include "Component.h"
#include "Transform.h"
#include "SceneContext.h"
#include "DrawData.h"

using Component = std::variant<
	Transform*,
	DrawData*
>;

using ComponentMap = std::unordered_map<std::type_index, Component>;

struct IActorType {
	virtual ~IActorType() = default;
	virtual void OnDraw() {}
	std::vector<ActorUUID> subActors;
	ActorUUID superActor = 0;
	bool hasSuperActor = false;
	SceneContext* sceneContext = nullptr;
	virtual inline ComponentMap GetComponents() { return {}; }
};

struct Container : public IActorType {};

using ActorPtr = std::unique_ptr<IActorType>&;

class Actor;
struct DeferredActor;

using DeferredPtr = std::unique_ptr<DeferredActor>;

struct DeferredActor {
	ActorUUID deferredUUID;
	Actor* parentHandlePtr;
	std::function<void(DeferredPtr& deferredRef, SceneContext*)> constructionFunction;
	DeferredActor(ActorUUID deferred, Actor* parent) : deferredUUID(deferred), parentHandlePtr(parent) {}
	inline void Make(DeferredPtr& deferredRef, SceneContext* context) { constructionFunction(deferredRef, context); }
};

class Actor {
public:
	Actor() = delete;
	Actor(const Actor& copy);

	template<ActorConcept T, typename... Args> Actor Add(Args&&... args) {
		ActorUUID subActorUUID = MakeUUID();
		bool deferringInit = sceneContext->initailizingFieldActors;
		Actor subActorHandle(sceneContext, subActorUUID);

		if (deferringInit) { AddDeferred<T>(subActorUUID, args...); return subActorHandle; }
		AddNow<T>(subActorUUID, args...); return subActorHandle;
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
	void Delete();
	inline bool IsInScene() const { return sceneContext->sceneActors.contains(selfUUID); }
	void DeleteAllSub();
private:
	friend class IScene;
	friend struct DeferredActor;
	Actor(SceneContext* context);

	ActorUUID selfUUID = 0;
	SceneContext* sceneContext = nullptr;

	ActorUUID MakeUUID() { return Random<ActorUUID>::Any(); }
	Actor(SceneContext* context, ActorUUID uuid) : sceneContext(context), selfUUID(uuid) {}
	ActorPtr GetActorDataPtr(ActorUUID uuid) const { return sceneContext->sceneActors[uuid]; }
	void ThrowIfFreed() const;

	constexpr ActorUUID GetUUID() { return selfUUID; }
	template<ActorConcept T, typename... Args> void AddDeferred(ActorUUID subActorUUID, Args&&... args) {
		DeferredPtr deferred = std::make_unique<DeferredActor>(subActorUUID, this);
		deferred->constructionFunction = [&](DeferredPtr& deferredRef, SceneContext* context) {
			ActorUUID deferredUUID = deferredRef->deferredUUID;
			deferredRef->parentHandlePtr->sceneContext = context;
			deferredRef->parentHandlePtr->AddNow<T>(deferredUUID, args...);
		};

		sceneContext->deferredActors.push_back(std::move(deferred));
	}
	template<ActorConcept T, typename... Args> void AddNow(ActorUUID subActorUUID, Args&&... args) {
		ActorPtr subActorData = GetActorDataPtr(subActorUUID);
		if constexpr (std::same_as<T, Container>) subActorData = std::make_unique<T>(args...);
		else subActorData = std::make_unique<T>(sceneContext, args...);

		subActorData->superActor = selfUUID;
		subActorData->hasSuperActor = true;

		ActorPtr selfData = GetActorDataPtr(selfUUID);
		selfData->subActors.push_back(subActorUUID);
	}
};