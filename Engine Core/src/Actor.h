#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include "Random.h"

using ActorUUID = uint64_t;

struct IActorType {
	virtual ~IActorType() = default;
	virtual void OnDraw() {}
	std::vector<ActorUUID> subActors;
	ActorUUID superActor = 0;
	bool hasSuperActor = false;
};

template<typename T>
concept ActorConcept = std::is_base_of_v<IActorType, T> && !std::is_same_v<IActorType, T>;

struct Container : public IActorType {
	Container() = default;
	~Container() { std::cout << "FREED CONTAINER" << std::endl; }
};

using SceneActors = std::unordered_map<ActorUUID, std::unique_ptr<IActorType>>;

class Actor {
public:
	Actor(SceneActors* actors);
	Actor() = delete;
	Actor(const Actor& copy);
	~Actor() { DeleteActor(false); }
	ActorUUID GetUUID() const { return selfUUID; }
	template<ActorConcept T, typename... Args> Actor Add(Args&&... args) {
		ActorUUID subActorUUID = MakeUUID();
		auto& subActorData = GetActorMapRef(subActorUUID);
		subActorData = std::make_unique<T>(args...);
		subActorData->superActor = selfUUID;
		subActorData->hasSuperActor = true;
		std::cout << "Creating: " << subActorUUID << std::endl;

		auto& selfData = GetActorMapRef(selfUUID);
		selfData->subActors.push_back(subActorUUID);
		Actor subActorHandle(sceneActors, subActorUUID);
		subActorHandle.deletableOnDestruction = true;
		return subActorHandle;
	}
	int GetSubCount() const;
	Actor operator[](int index) const;
	inline bool HasSuper() const { return GetActorMapRef(selfUUID)->hasSuperActor; }
	Actor GetSuper() const;
	inline void Delete() { DeleteActor(true); }
	inline bool IsInScene() const { return sceneActors->contains(selfUUID); }
	void DeleteAllSub();
private:
	Actor(SceneActors* actors, ActorUUID uuid) : sceneActors(actors), selfUUID(uuid) {}
	std::unique_ptr<IActorType>& GetActorMapRef(ActorUUID uuid) const { return (*sceneActors)[uuid]; }
	ActorUUID selfUUID;
	SceneActors* sceneActors = nullptr;
	ActorUUID MakeUUID() { return Random<ActorUUID>::Any(); }
	void DeleteActor(bool manualDeletion);
	void ThrowIfFreed() const;
	bool deletableOnDestruction = false;
};