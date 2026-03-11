#pragma once
#include <random>
#include <cstdint>
#include <type_traits>

namespace ThornEngine {

template<typename T>
concept Is32Bit = std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>;

template<typename T>
concept Is64Bit = std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>;

template<typename T>
concept IsReal = std::is_same_v<T, float> || std::is_same_v<T, double>;

template<typename T>
concept Generatable = Is32Bit<T> || Is64Bit<T> || IsReal<T>;

template<Generatable T = uint32_t>
class Random {
public:
	using MersenneTwister = std::conditional_t<Is32Bit<T>, std::mt19937, std::mt19937_64>;
	using RandomRange = std::conditional_t<IsReal<T>, std::uniform_real_distribution<T>, std::uniform_int_distribution<T>>;
	static T Range(T low, T high) { return RandomRange(low, high)(generationEngine); }
	static T Any() { return RandomRange()(generationEngine); }
private:
	static inline std::random_device seedGenerator;
	static inline MersenneTwister generationEngine = MersenneTwister(seedGenerator());
};

}