#include <Thorn.h>

enum class TextureID {
	TestTexture
};

enum class AudioID {
	TestAudio
};

class StartScene : public IScene {
	Actor sprite = Top<Sprite>();
	void OnStart() override {
		auto& comp = sprite.Get<Transform>();
		std::cout << comp << std::endl;
	}
};

int main(int argc, char* argv[]) {
	AppConfig config("Application Title", 800, 600);
	App app(config);
	app.CoreResources<
		TextureRes, TextureID,
		AudioRes, AudioID
	>();
	app.Run<StartScene>();
	return 0;
}