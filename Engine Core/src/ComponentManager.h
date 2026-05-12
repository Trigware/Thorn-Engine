#pragma once
#include <typeindex>
#include <unordered_map>
#include <memory>

namespace ThornEngine {

struct IComponentStorage {
	virtual ~IComponentStorage() = default;
	virtual void OnDraw() = 0;
};

template<typename T>
struct ComponentStorage : IComponentStorage {
	std::unordered_map<uint64_t, T> components;
	void OnDraw() override {
		for (auto it = components.begin(); it != components.end(); it++) {
			T& currentComponent = it->second;
			currentComponent.OnDraw();
		}
	}
};

using CompStorageMap = std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>>;

class ComponentManager {
public:
	template<typename T>
	void Bind(uint64_t actorID, T& component) {
		std::type_index storageType = typeid(T);
		if (!StorageAvailable<T>()) storages[storageType] = std::make_unique<ComponentStorage<T>>();
		ComponentStorage<T>& componentStorage = GetStorage<T>();
		componentStorage.components[actorID] = component;
	}

	template<typename T>
	void Unbind(uint64_t actorID) {
		bool hasComponent = Has<T>(actorID);
		if (!hasComponent) return;

		ComponentStorage<T>& componentStorage = GetStorage<T>();
		componentStorage.components.erase(actorID);
	}

	template<typename T>
	bool Has(uint64_t actorID) {
		if (!StorageAvailable<T>()) return false;

		ComponentStorage<T>& componentStorage = GetStorage<T>();
		bool hasComponent = componentStorage.components.contains(actorID);
		return hasComponent;
	}

	template<typename T>
	T& Get(uint64_t actorID) {
		bool hasComponent = Has<T>(actorID);
		if (!hasComponent) throw std::runtime_error("Attempted to get a component which is not available for this actor!");;

		ComponentStorage<T>& componentStorage = GetStorage<T>();
		return componentStorage.components.at(actorID);
	}

private:
	friend class IScene;
	template<typename T>
	bool StorageAvailable() {
		std::type_index storageType = typeid(T);
		bool storageAvailble = storages.contains(storageType);
		return storageAvailble;
	}

	template<typename T>
	ComponentStorage<T>& GetStorage() { 
		if (!StorageAvailable<T>()) throw std::runtime_error("Unable to return storage which is unavailable!");

		std::type_index storageType = typeid(T);
		std::unique_ptr<IComponentStorage>& compStorage = storages[storageType];
		ComponentStorage<T>* storagePtr = dynamic_cast<ComponentStorage<T>*>(compStorage.get());
		return *storagePtr;
	}

	CompStorageMap storages;
};

}