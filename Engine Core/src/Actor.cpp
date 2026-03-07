#include "Actor.h"

int Actor::GetSubCount() const {
	ThrowIfFreed();
	auto& selfData = GetActorDataPtr(selfUUID);
	return selfData->subActors.size();
}

Actor Actor::operator[](int index) const {
	ThrowIfFreed();
	int numberOfChildren = GetSubCount();
	if (index < 0 || index >= numberOfChildren) throw std::runtime_error("Attempted to access an invalid sub-actor!");
	auto& selfData = GetActorDataPtr(selfUUID);
	ActorUUID subActorUUID = selfData->subActors[index];
	Actor subActor(sceneContext, subActorUUID);
	return subActor;
}

Actor Actor::Super() const {
	ThrowIfFreed();
	if (!HasSuper()) throw std::runtime_error("Attempted to access a super-actor from scene root!");
	ActorUUID superUUID = GetActorDataPtr(selfUUID)->superActor;
	Actor superActor(sceneContext, superUUID);
	return superActor;
}

void Actor::DeleteActor(bool manualDeletion) {
	bool willDelete = (manualDeletion || deletableOnDestruction) && IsInScene();
	if (!willDelete) return;
	DeleteAllSub();
	GetActorDataPtr(selfUUID).reset();
	sceneContext->sceneActors.erase(selfUUID);
}

Actor::Actor(SceneContext* context) : sceneContext(context), selfUUID(MakeUUID()) {
	ActorPtr actorPtr = GetActorDataPtr(selfUUID);
	actorPtr = std::make_unique<Container>();
	actorPtr->appContext = context->appContext;
	deletableOnDestruction = false;
}

Actor::Actor(const Actor& copy) {
	*this = Actor(copy.sceneContext, copy.selfUUID);
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

Actor::~Actor() { DeleteActor(false); }
void Actor::Delete() { DeleteActor(true); }