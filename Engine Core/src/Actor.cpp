#include "Actor.h"

namespace ThornEngine {

void DeferredActor::Make(DeferredPtr& deferredPtr, SceneContext* context) {
	constructionFunction(deferredPtr, context);
	for (int i = 0; i < bindingFunctions.size(); i++) {
		BindingFunction& bindFunc = bindingFunctions[i];
		bindFunc.function(deferredPtr, bindFunc.isUnbinding);
	}
}

int Actor::GetSubCount() const {
	ThrowIfFreed();
	return GetData().subActors.size();
}

Actor Actor::operator[](int index) const {
	ThrowIfFreed();
	int numberOfChildren = GetSubCount();
	if (index < 0 || index >= numberOfChildren) throw std::runtime_error("Attempted to access an invalid sub-actor!");

	ActorData& selfData = GetData();
	ActorID subActorID = selfData.subActors[index];
	Actor subActor(sceneContext, subActorID);
	return subActor;
}

Actor Actor::Super() const {
	ThrowIfFreed();
	if (!HasSuper()) throw std::runtime_error("Attempted to access a super-actor from scene root!");
	ActorID superID = GetData().superActor;
	Actor superActor(sceneContext, superID);
	return superActor;
}

void Actor::Delete() {
	ThrowIfFreed();
	DeleteAllSub();
	sceneContext->sceneActors.erase(selfID);
}

Actor::Actor(SceneContext* context) : sceneContext(context), selfID(MakeID()) {
	ActorData& actorRef = GetData();
	actorRef = ActorData(context);
}

Actor::Actor(const Actor& copy) { *this = Actor(copy.sceneContext, copy.selfID); }

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

}