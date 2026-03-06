#include <Thorn.h>

enum class TextureID { Texture1, Texture2, Texture3 };

enum class AudioID {
	TestAudio
};

class StartScene : public IScene {
	Actor sprite = Top<Sprite>();
	void OnStart() override {
	}
};

int main(int argc, char* argv[]) {
	App app("Application", 800, 600);
	app.CoreResources<
		TextureRes, TextureID
	>();
	return app.GetExitCode();
}