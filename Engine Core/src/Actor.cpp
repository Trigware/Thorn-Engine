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
	return GetSelfDataRef().subActors.size();
}

Actor Actor::operator[](int index) const {
	ThrowIfFreed();
	int numberOfChildren = GetSubCount();
	if (index < 0 || index >= numberOfChildren) throw std::runtime_error("Attempted to access an invalid sub-actor!");

	ActorData& selfData = GetSelfDataRef();
	ActorUUID subActorUUID = selfData.subActors[index];
	Actor subActor(sceneContext, subActorUUID);
	return subActor;
}

Actor Actor::Super() const {
	ThrowIfFreed();
	if (!HasSuper()) throw std::runtime_error("Attempted to access a super-actor from scene root!");
	ActorUUID superUUID = GetSelfDataRef().superActor;
	Actor superActor(sceneContext, superUUID);
	return superActor;
}

void Actor::Delete() {
	ThrowIfFreed();
	DeleteAllSub();
	sceneContext->sceneActors.erase(selfUUID);
}

Actor::Actor(SceneContext* context) : sceneContext(context), selfUUID(MakeUUID()) {
	ActorData& actorRef = GetSelfDataRef();
	actorRef = ActorData(context);
}

Actor::Actor(const Actor& copy) { *this = Actor(copy.sceneContext, copy.selfUUID); }

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

void ActorData::OnDraw() {
	for (auto it = components.begin(); it != components.end(); it++) {
		std::unique_ptr<IComponent>& currentComponent = it->second;
		currentComponent->OnDraw();
	}
}

}