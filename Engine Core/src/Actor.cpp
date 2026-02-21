#include "Actor.h"

int Actor::GetSubCount() const {
	ThrowIfFreed();
	auto& selfData = GetActorMapRef(selfUUID);
	return selfData->subActors.size();
}

Actor Actor::operator[](int index) const {
	ThrowIfFreed();
	int numberOfChildren = GetSubCount();
	if (index < 0 || index >= numberOfChildren) throw std::runtime_error("Attempted to access an invalid sub-actor!");
	auto& selfData = GetActorMapRef(selfUUID);
	ActorUUID subActorUUID = selfData->subActors[index];
	Actor subActor(sceneActors, subActorUUID);
	return subActor;
}

Actor Actor::GetSuper() const {
	ThrowIfFreed();
	if (!HasSuper()) throw std::runtime_error("Attempted to access a super-actor from scene root!");
	ActorUUID superUUID = GetActorMapRef(selfUUID)->superActor;
	Actor superActor(sceneActors, superUUID);
	return superActor;
}

void Actor::DeleteActor(bool manualDeletion) {
	bool willDelete = (manualDeletion || deletableOnDestruction) && IsInScene();
	if (!willDelete) return;
	DeleteAllSub();
	GetActorMapRef(selfUUID).reset();
	std::cout << "Deleted: " << selfUUID << std::endl;
	sceneActors->erase(selfUUID);
}

Actor::Actor(SceneActors* actors) : sceneActors(actors), selfUUID(MakeUUID()) {
	GetActorMapRef(selfUUID) = std::make_unique<Container>();
	deletableOnDestruction = true;
	std::cout << "ROOT: " << selfUUID << std::endl;
}

Actor::Actor(const Actor& copy) {
	*this = Actor(copy.sceneActors, copy.selfUUID);
	deletableOnDestruction = false;
}

void Actor::ThrowIfFreed() const {
	if (IsInScene()) return;
	throw std::runtime_error("Attempted to access memory from a previously-freed actor!");
}

void Actor::DeleteAllSub() {
	int subCount = GetSubCount();
	for (int i = 0; i < subCount; i++) {
		Actor subActor = (*this)[i];
		subActor.Delete();
	}
}