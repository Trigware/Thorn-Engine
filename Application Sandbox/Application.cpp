#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };

class StartScene : public IScene {
	Actor spr = Top<Empty>().Bind<Transform>().Bind<DrawData>(TextureID::TimerClock);
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