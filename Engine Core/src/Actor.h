#pragma once
#include <iostream>
#include <memory>
#include <typeindex>
#include <variant>
#include <utility>
#include <functional>
#include <unordered_set>
#include "Random.h"
#include "Component.h"
#include "SceneContext.h"

namespace ThornEngine {

class ActorData {
public:
	std::vector<ActorUUID> subActors;
	ActorUUID superActor = 0;
	bool hasSuperActor = false;
	ComponentMap components;
	ComponentSet nonRemovableComponents = {};

	ActorData(SceneContext* context) : sceneContext(context) {}
	ActorData() = default;
	void OnDraw();
private:
	SceneContext* sceneContext = nullptr;
};

class Actor;
struct DeferredActor;

using DeferredPtr = std::unique_ptr<DeferredActor>;

struct Empty : public IActorType {
	ComponentSet GetSet() override { return {}; }
};

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

/* @brief Reference to an entity with components belonging to the scene tree.
* 
* It's a light-weight and safely-copyable type that stores a 64-bit randomly-generated identifier with a pointer to scene information.
*/
class Actor {
public:
	Actor() = delete;
	Actor(const Actor& copy);
	Actor(SceneContext* context, ActorUUID uuid) : sceneContext(context), selfUUID(uuid) {}

	template<ActorTypeConcept T, typename... Args> Actor Add(Args&&... args) {
		ActorUUID subActorUUID = MakeUUID();
		bool deferringInit = sceneContext->initailizingFieldActors;
		Actor subActorHandle(sceneContext, subActorUUID);

		if (deferringInit) { AddDeferred<T>(subActorUUID, args...); return subActorHandle; }
		AddNow<T>(subActorUUID, args...); return subActorHandle;
	}
	template<ComponentConcept T> T& Get() {
		ActorData& actorData = GetData();
		std::type_index componentType = typeid(T);
		if (!Has<T>()) throw std::runtime_error("Attempted to get a component which is not available for this actor!");

		std::unique_ptr<IComponent>& genericComponentPtr = actorData.components[componentType];
		T* componentPtr = dynamic_cast<T*>(genericComponentPtr.get());
		return *componentPtr;
	}
	template<ComponentConcept T, typename... Args> Actor& Bind(Args&&... args) {
		ActorData& actorData = GetData();
		if (Has<T>()) throw std::runtime_error("Attempted to bind a component of a type which has already been binded!");

		bool deferringInit = sceneContext->initailizingFieldActors;
		if (deferringInit) { HandleBindingDeferred<T>(false, args...); return *this; }

		constexpr bool hasSceneContext = std::is_constructible_v<T, SceneContext*, Args...>,
			hasValidParameters = std::is_constructible_v<T, Args...>;

		std::unique_ptr<T> componentPtr;
		if constexpr (hasSceneContext) componentPtr = std::make_unique<T>(sceneContext, args...);
		else {
			static_assert(hasValidParameters, "Attempted to bind a component to an actor, however none of wanted component's constructors match the given argument types!");
			componentPtr = std::make_unique<T>(args...);
		}

		componentPtr->linkedActorUUID = selfUUID;
		std::type_index componentType = typeid(T);
		actorData.components.emplace(componentType, std::move(componentPtr));
		return *this;
	}
	template<ComponentConcept T, ComponentConcept... Other> Actor& BindMore() {
		Bind<T>();
		if constexpr (sizeof...(Other) > 0) return BindMore<Other...>();
		return *this;
	}
	template<ComponentConcept... Args> bool Has() {
		ActorData& actorData = GetData();
		return (actorData.components.contains(typeid(Args)) && ...);
	}
	template<ComponentConcept T, ComponentConcept... Other> Actor& Unbind() {
		ActorData& actorData = GetData();
		std::type_index typeToBeRemoved = typeid(T);

		bool deferringInit = sceneContext->initailizingFieldActors;
		if (deferringInit) HandleBindingDeferred<T>(true);

		bool hasComponent = Has<T>();
		if (hasComponent) actorData.components.erase(typeToBeRemoved);
		if (actorData.nonRemovableComponents.contains(typeToBeRemoved))
			throw std::runtime_error("Attempted to unbind a component which is associated with the actor type that binded this actor!");

		if constexpr (sizeof...(Other) > 0) return Unbind<Other...>();
		return *this;
	}

	int GetSubCount() const;
	Actor operator[](int index) const;
	inline bool HasSuper() const { return GetData().hasSuperActor; }
	Actor Super() const;
	void Delete();
	inline bool IsInScene() const { return sceneContext->sceneActors.contains(selfUUID); }
	void DeleteAllSub();
private:
	friend struct DeferredActor;
	friend class IScene;
	friend class IComponent;
	Actor(SceneContext* context);

	ActorUUID selfUUID = 0;
	SceneContext* sceneContext = nullptr;

	ActorUUID MakeUUID() { return Random<ActorUUID>::Any(); }
	inline ActorData& GetActorData(ActorUUID uuid) const { return sceneContext->sceneActors[uuid]; }
	inline ActorData& GetData() const { return GetActorData(selfUUID); }
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
		ActorData& subActorData = GetActorData(subActorUUID);
		subActorData = ActorData(sceneContext);

		constexpr bool isEmptyActorType = std::is_same_v<T, Empty>, actorTypeConstructable = std::is_constructible_v<T, Actor, Args...> || isEmptyActorType && sizeof...(Args) == 0;
		static_assert(actorTypeConstructable, "Attempted to create an actor based on an actor type, however none of wanted actor type's constructors matches given argument types!");
		if constexpr (!isEmptyActorType) {
			Actor subActorHandle(sceneContext, subActorUUID);
			T actorBuilder = T(subActorHandle, args...);
			subActorHandle.GetData().nonRemovableComponents = std::move(actorBuilder.GetSet());
		}

		subActorData.superActor = selfUUID;
		subActorData.hasSuperActor = true;

		ActorData& selfData = GetActorData(selfUUID);
		selfData.subActors.push_back(subActorUUID);
	}
	inline DeferredPtr& GetLatestDeferred() { return sceneContext->deferredActors[sceneContext->deferredActors.size() - 1]; }
};

inline Actor IComponent::GetOwner() { return Actor(sceneContext, linkedActorUUID); }

}