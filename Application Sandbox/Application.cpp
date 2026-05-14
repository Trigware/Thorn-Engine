#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };
enum class ActionID { MoveLeft, MoveRight, MoveUp, MoveDown };

class StartScene : public TH::IScene {
	TH::Actor clock = Top<TH::Sprite>(TextureID::TimerClock), grid = Top<TH::Sprite>(TextureID::GridTiles);
	TH::TForm &tClock = clock.Get<TH::TForm>(), &tGrid = grid.Get<TH::TForm>();

	void OnDraw() override {
		tClock.pos.x += 10;
		tGrid.pos.y += 10;
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