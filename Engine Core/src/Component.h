#pragma once
#include <concepts>
#include <typeindex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "Assets.h"
#include "SceneContext.h"
#include "AppContext.h"
#include "Random.h"

namespace ThornEngine {

class Actor;

/* @brief Interface for engine core and custom-made components.
* 
* Components inheriting this interface must have a default constructor.
* 
* Gets some access to key engine features such as resources and the SceneContext.
* Inproper use of these features could create unexpected behaviour.
*/
class IComponent {
public:
	virtual ~IComponent() = default;
	IComponent(const IComponent&) = delete;
	IComponent() = default;

	virtual void OnDraw() {}
	IComponent(SceneContext* context) : sceneContext(context) { }
	inline Actor GetOwner();
	int identifier = Random<uint64_t>::Any();
protected:
	template<Resource Res, Enum ID>
	ConditionalAsset<Res>* GetAsset(ID identifier) { return sceneContext->appContext->GetAsset<Res>(identifier); }
	SceneContext* sceneContext = nullptr;
private:
	friend class Actor;
	ActorID linkedActorID = 0;
};

using ComponentSet = std::unordered_set<std::type_index>;

class IActorType;

template<typename T> concept ComponentConcept = std::is_base_of_v<IComponent, T> && !std::is_same_v<IComponent, T>;
template<typename T> concept ActorTypeConcept = std::is_base_of_v<IActorType, T> && !std::is_same_v<IActorType, T> &&
	requires {
		{ T::GetSet() } -> std::same_as<ComponentSet>;
	};

/* @brief Interface for engine core and custom-made actor types.
* 
* Derived classes must implement a static GetSet function which returns ComponentSet.
* You can create a component set using the CompSet function and specified the component or different actor type as template parameters.
*/
class IActorType {
public:
	virtual ~IActorType() = default;
	template<typename... T>
	static ComponentSet CompSet() {
		ComponentSet usedSet;
		CompSet<T...>(usedSet);
		return usedSet;
	}
private:
	template<typename T, typename... Other>
	static void CompSet(ComponentSet& currentSet) {
		constexpr bool isComponent = ComponentConcept<T>, isActorType = ActorTypeConcept<T>;
		static_assert(isComponent || isActorType, "Attempted to build a component set with a type which is neither component or an actor type!");

		if constexpr (isActorType) {
			ComponentSet actorTypeSet = T::GetSet();
			currentSet.insert(actorTypeSet.begin(), actorTypeSet.end());
		}
		else currentSet.insert(typeid(T));

		if constexpr (sizeof...(Other) > 0) CompSet<Other...>(currentSet);
	}
};

}