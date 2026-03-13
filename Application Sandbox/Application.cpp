#include <Thorn.h>

enum class TextureID { TimerClock, GridTiles };
class StartScene : public TH::IScene {};

int main(int argc, char* argv[]) {
	TH::App app("Application", 800, 600);
	app.CoreResources<
		TH::TextureRes, TextureID
	>();
	app.Run<StartScene>();
	return app.GetExitCode();
}