#include <App.h>

enum class TextureID {
	TestTexture
};

enum class AudioID {
	TestAudio
};

class StartScene : public IScene {
	Actor test = Root().Add<Container>();
	void OnStart() override {
		Actor sub1 = test.Add<Container>(), sub2 = test.Add<Container>(), subsub = sub1.Add<Container>();
		test.DeleteAllSub();
		std::cout << "TEST: " << test.IsInScene() << ", SUB1: " << sub1.GetSuper().GetUUID() << ", SUBSUB: " << subsub.IsInScene() << std::endl;
	}
};

int main(int argc, char* argv[]) {
	AppConfig config("Application Title", 800, 600);
	App app(config);
	app.CoreResources<
		TextureRes, TextureID,
		AudioRes, AudioID
	>();
	app.Run<StartScene>();
	return 0;
}