#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };

enum class AudioID {
	TestAudio
};

class StartScene : public IScene {
	Actor clock = Top<Sprite>(TextureID::TimerClock);
	void OnStart() override {
		clock.Get<Transform>().pos = V2I(300);
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