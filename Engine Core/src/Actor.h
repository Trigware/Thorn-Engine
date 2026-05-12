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
	std::vector<ActorID> subActors;
	ActorID superActor = 0;
	bool hasSuperActor = false;
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
	ActorID deferredID;
	Actor* parentHandlePtr;
	std::function<void(DeferredPtr&, SceneContext*)> constructionFunction;
	std::vector<BindingFunction> bindingFunctions;
	DeferredActor(ActorID deferred, Actor* parent) : deferredID(deferred), parentHandlePtr(parent) {}
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
	Actor(SceneContext* context, ActorID id) : sceneContext(context), selfID(id) {}

	template<ActorTypeConcept T, typename... Args> Actor Add(Args&&... args) {
		ActorID subActorID = MakeID();
		bool deferringInit = sceneContext->initailizingFieldActors;
		Actor subActorHandle(sceneContext, subActorID);

		if (deferringInit) { AddDeferred<T>(subActorID, args...); return subActorHandle; }
		AddNow<T>(subActorID, args...); return subActorHandle;
	}

	template<ComponentConcept T> T& Get() {
		bool isInit = sceneContext->initailizingFieldActors;
		if (isInit) return GetDeferredComponent<T>();

		T& componentRef = sceneContext->componentManager.Get<T>(selfID);
		return componentRef;
	}

	template<ComponentConcept T, typename... Args> Actor& Bind(Args&&... args) {
		if (Has<T>()) throw std::runtime_error("Attempted to bind a component of a type which has already been bound!");

		bool deferringInit = sceneContext->initailizingFieldActors;
		if (deferringInit) { HandleBindingDeferred<T>(false, args...); return *this; }

		constexpr bool hasSceneContext = std::is_constructible_v<T, SceneContext*, Args...>,
			hasValidParameters = std::is_constructible_v<T, Args...>;

		T boundComponent;
		if constexpr (hasSceneContext) boundComponent = T(sceneContext, args...);
		else {
			static_assert(hasValidParameters, "Attempted to bind a component to an actor, however none of wanted component's constructors match the given argument types!");
			boundComponent = T(args...);
		}

		boundComponent.linkedActorID = selfID;
		sceneContext->componentManager.Bind<T>(selfID, boundComponent);

		return *this;
	}

	template<ComponentConcept T, ComponentConcept... Other> Actor& BindMore() {
		Bind<T>();
		if constexpr (sizeof...(Other) > 0) return BindMore<Other...>();
		return *this;
	}

	template<ComponentConcept T, ComponentConcept... Other>
	bool Has() {
		bool hasCurrent = sceneContext->componentManager.Has<T>(selfID);
		if (!hasCurrent) return false;
		if constexpr (sizeof...(Other) > 0) return Has<Other...>();
		return true;
	}

	template<ComponentConcept T, ComponentConcept... Other> Actor& Unbind() {
		ActorData& actorData = GetData();
		std::type_index typeToBeRemoved = typeid(T);

		bool deferringInit = sceneContext->initailizingFieldActors;
		if (deferringInit) HandleBindingDeferred<T>(true);

		if (actorData.nonRemovableComponents.contains(typeToBeRemoved))
			throw std::runtime_error("Attempted to unbind a component which is associated with the actor type that binded this actor!");
		
		sceneContext->componentManager.Unbind<T>(selfID);

		if constexpr (sizeof...(Other) > 0) return Unbind<Other...>();
		return *this;
	}

	int GetSubCount() const;
	Actor operator[](int index) const;
	inline bool HasSuper() const { return GetData().hasSuperActor; }
	Actor Super() const;
	void Delete();
	inline bool IsInScene() const { return sceneContext->sceneActors.contains(selfID); }
	void DeleteAllSub();
	constexpr ActorID GetID() { return selfID; }

private:
	friend struct DeferredActor;
	friend class IScene;
	friend class IComponent;
	Actor(SceneContext* context);

	ActorID selfID = 0;
	SceneContext* sceneContext = nullptr;

	ActorID MakeID() { return Random<ActorID>::Any(); }
	inline ActorData& GetActorData(ActorID id) const { return sceneContext->sceneActors[id]; }
	inline ActorData& GetData() const { return GetActorData(selfID); }
	void ThrowIfFreed() const;

	template<ActorTypeConcept T, typename... Args> void AddDeferred(ActorID subActorID, Args&&... args) {
		DeferredPtr deferred = std::make_unique<DeferredActor>(subActorID, this);
		deferred->constructionFunction = [...args = std::forward<Args>(args)](DeferredPtr& deferredRef, SceneContext* context) {
			ActorID deferredID = deferredRef->deferredID;
			deferredRef->parentHandlePtr->sceneContext = context;
			deferredRef->parentHandlePtr->AddNow<T>(deferredID, args...);
		};

		sceneContext->deferredActors[subActorID] = std::move(deferred);
	}

	template<ComponentConcept T, typename... Args> void HandleBindingDeferred(bool isUnbinding, Args&&... args) {
		DeferredPtr& deferred = GetDeferred();
		deferred->bindingFunctions.emplace_back([...args = std::forward<Args>(args)](DeferredPtr& deferredRef, bool unbinding) {
			if (unbinding) deferredRef->parentHandlePtr->Unbind<T>();
			else deferredRef->parentHandlePtr->Bind<T>(args...);
		}, isUnbinding);
	}

	template<ComponentConcept T> T& GetDeferredComponent() {
		T t; return t;
	}

	template<ActorTypeConcept T, typename... Args> void AddNow(ActorID subActorID, Args&&... args) {
		ActorData& subActorData = GetActorData(subActorID);
		subActorData = ActorData(sceneContext);

		constexpr bool isEmptyActorType = std::is_same_v<T, Empty>, actorTypeConstructable = std::is_constructible_v<T, Actor, Args...> || isEmptyActorType && sizeof...(Args) == 0;
		static_assert(actorTypeConstructable, "Attempted to create an actor based on an actor type, however none of wanted actor type's constructors matches given argument types!");
		if constexpr (!isEmptyActorType) {
			Actor subActorHandle(sceneContext, subActorID);
			T actorBuilder = T(subActorHandle, args...);
			subActorHandle.GetData().nonRemovableComponents = std::move(actorBuilder.GetSet());
		}

		subActorData.superActor = selfID;
		subActorData.hasSuperActor = true;

		ActorData& selfData = GetActorData(selfID);
		selfData.subActors.push_back(subActorID);
	}
	inline DeferredPtr& GetDeferred() { return sceneContext->deferredActors[selfID]; }
};

inline Actor IComponent::GetOwner() { return Actor(sceneContext, linkedActorID); }

}