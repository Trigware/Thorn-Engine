#pragma once
#include <concepts>
#include <typeindex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Assets.h"
#include "SceneContext.h"

namespace ThornEngine {

class Actor;

/* @brief Interface for engine core and custom-made components.
* 
* Components inheriting this interface must have a default constructor.
* 
* Gets direct access to key engine features such as resources and the SceneContext.
* Inproper use of these features can could unexpected behaviour.
*/
class IComponent {
public:
	virtual ~IComponent() = default;
	IComponent() = default;
	virtual void OnDraw() {}
	SceneContext* sceneContext = nullptr;
	IComponent(SceneContext* context) : sceneContext(context) { }
	inline Actor GetOwner();
protected:
	template<Resource Res, Enum ID>
	ConditionalAsset<Res>* GetAsset(ID identifier) { return sceneContext->appContext->GetAsset<Res>(identifier); }
private:
	friend class Actor;
	ActorUUID linkedActorUUID = 0;
};

using ComponentMap = std::unordered_map<std::type_index, std::unique_ptr<IComponent>>;
using ComponentSet = std::unordered_set<std::type_index>;

class IActorType;

template<typename T> concept ComponentConcept = std::is_base_of_v<IComponent, T> && !std::is_same_v<IComponent, T>;
template<typename T> concept ActorTypeConcept = std::is_base_of_v<IActorType, T> && !std::is_same_v<IActorType, T>;

/* @brief Interface for engine core and custom-made actor types.
* 
* Derived classes must override the GetSet function that ensures that the components in the returned set always belong to an actor created from that actor type.
* 
* A sub-actor must be created by an using the actor type's constructors.
*/
class IActorType {
public:
	virtual ~IActorType() = default;
	virtual ComponentSet GetSet() = 0;
	template<typename... T>
	ComponentSet CompSet() {
		ComponentSet usedSet;
		CompSet<T...>(usedSet);
		return usedSet;
	}
private:
	template<typename T, typename... Other>
	void CompSet(ComponentSet& currentSet) {
		constexpr bool isComponent = ComponentConcept<T>, isActorType = ActorTypeConcept<T>;
		static_assert(isComponent || isActorType, "Attempted to build a component set with a type which is neither component or an actor type!");

		if constexpr (isActorType) {
			ComponentSet actorTypeSet = T().GetSet();
			currentSet.insert(actorTypeSet.begin(), actorTypeSet.end());
		}
		else currentSet.insert(typeid(T));

		if constexpr (sizeof...(Other) > 0) CompSet<Other...>(currentSet);
	}
};

}