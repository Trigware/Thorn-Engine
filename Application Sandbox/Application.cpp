#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };

class StartScene : public IScene {
	Actor spr = Top<Empty>().Bind<DrawData>();
	void OnStart() override {
		std::cout << spr.Has<DrawData>() << std::endl;
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