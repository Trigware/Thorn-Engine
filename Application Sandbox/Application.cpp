#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };

enum class AudioID {
	TestAudio
};

class StartScene : public IScene {
	using IScene::IScene;
	Actor container = Top<Container>();
	void OnDraw() override {
	}
};

int main(int argc, char* argv[]) {
	App app("Application", 800, 600);
	app.CoreResources<
		TextureRes, TextureID
	>();
	app.Run<StartScene>();
	return app.GetExitCode();
}