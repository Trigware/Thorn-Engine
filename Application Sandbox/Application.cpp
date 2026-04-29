#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };
enum class ActionID { MoveLeft, MoveRight, MoveUp, MoveDown };

class StartScene : public TH::IScene {
	TH::Actor spr = Top<TH::Sprite>(TextureID::TimerClock);
	void OnDraw() override {
		if (Input.IsActionTapped(ActionID::MoveRight)) std::cout << "TAP" << std::endl;
		if (Input.IsActionHeld(ActionID::MoveRight)) std::cout << "HOLD" << std::endl;
		if (Input.IsActionReleased(ActionID::MoveRight)) std::cout << "RELEASED" << std::endl;
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