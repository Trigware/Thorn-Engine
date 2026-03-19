#include "ResType.h"

namespace ThornEngine {

ResTypeData::ResTypeData(ResType resType) {
	switch (resType) {
		case ResType::Texture: *this = ResTypeData("Textures", "png"); break;
	}
}

}