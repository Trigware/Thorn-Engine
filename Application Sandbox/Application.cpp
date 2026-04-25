#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };
enum class ActionID { MoveLeft, MoveRight, MoveUp, MoveDown };

class StartScene : public TH::IScene {
	TH::Actor spr = Top<TH::Sprite>(TextureID::TimerClock);
	void OnDraw() override {

	}
};

int main(int argc, char* argv[]) {
	TH::App app("Application", 800, 600);
	app.CoreResources<
		TH::TextureRes, TextureID,
		TH::ActionRes, ActionID
	>();
	app.Run<StartScene>();
	return app.GetExitCode();
}