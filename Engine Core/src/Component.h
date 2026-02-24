#pragma once
#include <concepts>
#include <typeindex>
#include <string>

struct IComponent { virtual ~IComponent() = default; };

template<typename T>
concept IsComponent = std::is_base_of_v<IComponent, T> && !std::is_same_v<IComponent, T>;