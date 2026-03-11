#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };

class StartScene : public IScene {
	Actor spr = Top<Sprite>(TextureID::TimerClock);
	void OnDraw() override {
		spr.Get<Transform>().pos.x++;
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