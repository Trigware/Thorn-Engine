#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };
enum class ActionID { MoveLeft, MoveRight, MoveUp, MoveDown };

class StartScene : public TH::IScene {
	TH::Actor spr = Top<TH::Sprite>(TextureID::TimerClock);
	void OnDraw() override {
		TH::V2F moveDir = TH::V2F::Zero;
		if (Input.IsActionHeld(ActionID::MoveLeft)) moveDir.x--;
		if (Input.IsActionHeld(ActionID::MoveRight)) moveDir.x++;
		if (Input.IsActionHeld(ActionID::MoveUp)) moveDir.y--;
		if (Input.IsActionHeld(ActionID::MoveDown)) moveDir.y++;
		spr.Get<TH::TForm>().pos += moveDir.GetNormalized() * 5;
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