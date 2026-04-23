#include <type_traits>
#include <typeindex>
#include <string>
#include "Utils.h"

namespace ThornEngine {

enum class ResType {
	Texture,
	Action,
	Counter
};

using TextureRes = std::integral_constant<ResType, ResType::Texture>;
using ActionRes = std::integral_constant<ResType, ResType::Action>;

template<typename T>
concept Resource =
	requires { T::value; } &&
	std::is_same_v<T, std::integral_constant<ResType, T::value>> &&
	static_cast<int>(T::value) >= 0 && T::value < ResType::Counter;

struct ResTypeData {
	ResTypeData(ResType resType);
	ResTypeData(const std::string& dir, const std::string& defExtension = "") : defaultExtension(defExtension) {
		assetDir = dir + "/";
		metadataFile = "Meta/" + StrUtils::ToLower(dir) + ".meta";
	}
	std::string assetDir = "", metadataFile = "", defaultExtension = "";
};

}