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

class ActorData {
public:
	std::vector<ActorUUID> subActors;
	ActorUUID superActor = 0;
	bool hasSuperActor = false;
	ComponentMap components;

	ActorData(SceneContext* context) : sceneContext(context) {}
	ActorData() = default;
	void OnDraw();

private:
	SceneContext* sceneContext = nullptr;
};

class Actor;
struct DeferredActor;

using DeferredPtr = std::unique_ptr<DeferredActor>;

struct Empty : public IActorType {};

using BindFunctionType = std::function<void(DeferredPtr&, bool)>;

struct BindingFunction {
	BindFunctionType function;
	bool isUnbinding = false;
	BindingFunction(const BindFunctionType& bindFunc, bool unbinding) : function(bindFunc), isUnbinding(unbinding) {}
	BindingFunction() = default;
};

struct DeferredActor {
	ActorUUID deferredUUID;
	Actor* parentHandlePtr;
	std::function<void(DeferredPtr&, SceneContext*)> constructionFunction;
	std::vector<BindingFunction> bindingFunctions;
	DeferredActor(ActorUUID deferred, Actor* parent) : deferredUUID(deferred), parentHandlePtr(parent) {}
	void Make(DeferredPtr& deferredRef, SceneContext* context);
};

class Actor {
public:
	Actor() = delete;
	Actor(const Actor& copy);

	template<ActorTypeConcept T, typename... Args> Actor Add(Args&&... args) {
		ActorUUID subActorUUID = MakeUUID();
		bool deferringInit = sceneContext->initailizingFieldActors;
		Actor subActorHandle(sceneContext, subActorUUID);

		if (deferringInit) { AddDeferred<T>(subActorUUID, args...); return subActorHandle; }
		AddNow<T>(subActorUUID, args...); return subActorHandle;
	}
	template<ComponentConcept T> T& Get() {
		ActorData& actorData = GetActorDataRef(selfUUID);
		std::type_index componentType = typeid(T);
		if (!Has<T>()) throw std::runtime_error("Attempted to get a component which is not available for this actor!");

		std::unique_ptr<IComponent>& genericComponent = actorData->components[componentType];
		T* componentPtr = std::get<T*>(genericComponent);
		return *componentPtr;
	}
	template<ComponentConcept T, typename... Args> Actor& Bind(Args&&... args) {
		ActorData& actorData = GetActorDataRef(selfUUID);
		if (Has<T>()) throw std::runtime_error("Attempted to bind a component of a type which has already been binded!");

		bool deferringInit = sceneContext->initailizingFieldActors;
		if (deferringInit) { HandleBindingDeferred<T>(false, args...); return *this; }

		std::type_index componentType = typeid(T);
		actorData.components.emplace(componentType, std::make_unique<T>(args...));
		return *this;
	}
	template<ComponentConcept T, ComponentConcept... Other> Actor& BindMore() {
		Bind<T>();
		if constexpr (sizeof...(Other) > 0) return BindMore<Other...>();
		return *this;
	}
	template<ComponentConcept... Args> bool Has() {
		ActorData& actorData = GetActorDataRef(selfUUID);
		return (actorData.components.contains(typeid(Args)) && ...);
	}
	template<ComponentConcept T, ComponentConcept... Other> Actor& Unbind() {
		ActorData& actorData = GetActorDataRef(selfUUID);
		std::type_index typeToBeRemoved = typeid(T);

		bool deferringInit = sceneContext->initailizingFieldActors;
		if (deferringInit) HandleBindingDeferred<T>(true);
		else if (Has<T>()) actorData.components.erase(typeToBeRemoved);

		if constexpr (sizeof...(Other) > 0) return Unbind<Other...>();
		return *this;
	}

	int GetSubCount() const;
	Actor operator[](int index) const;
	inline bool HasSuper() const { return GetActorDataRef(selfUUID).hasSuperActor; }
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
	ActorData& GetActorDataRef(ActorUUID uuid) const { return sceneContext->sceneActors[uuid]; }
	void ThrowIfFreed() const;

	constexpr ActorUUID GetUUID() { return selfUUID; }
	template<ActorTypeConcept T, typename... Args> void AddDeferred(ActorUUID subActorUUID, Args&&... args) {
		DeferredPtr deferred = std::make_unique<DeferredActor>(subActorUUID, this);
		deferred->constructionFunction = [&](DeferredPtr& deferredRef, SceneContext* context) {
			ActorUUID deferredUUID = deferredRef->deferredUUID;
			deferredRef->parentHandlePtr->sceneContext = context;
			deferredRef->parentHandlePtr->AddNow<T>(deferredUUID, args...);
		};

		sceneContext->deferredActors.push_back(std::move(deferred));
	}

	template<ComponentConcept T, typename... Args> void HandleBindingDeferred(bool isUnbinding, Args&&... args) {
		DeferredPtr& deferred = GetLatestDeferred();
		deferred->bindingFunctions.emplace_back([&](DeferredPtr& deferredRef, bool unbinding) {
			if (unbinding) deferredRef->parentHandlePtr->Unbind<T>();
			else deferredRef->parentHandlePtr->Bind<T>(args...);
		}, isUnbinding);
	}

	template<ActorTypeConcept T, typename... Args> void AddNow(ActorUUID subActorUUID, Args&&... args) {
		ActorData& subActorData = GetActorDataRef(subActorUUID);
		subActorData = ActorData(sceneContext);

		subActorData.superActor = selfUUID;
		subActorData.hasSuperActor = true;

		ActorData& selfData = GetActorDataRef(selfUUID);
		selfData.subActors.push_back(subActorUUID);
	}
	inline DeferredPtr& GetLatestDeferred() { return sceneContext->deferredActors[sceneContext->deferredActors.size() - 1]; }
};