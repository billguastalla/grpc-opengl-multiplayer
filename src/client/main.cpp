#include "multiplayer_scene_client.h"
#include "graphics.h"

#include <GLFW/glfw3.h>

void client_test() {
	MultiplayerSceneClient guide{
	grpc::CreateChannel("localhost:50051",
						grpc::InsecureChannelCredentials()) };

	std::cout << "-------------- MultiplayerScene --------------" << std::endl;
	guide.PrintEntities();
	guide.ChangeColour("5", 0);
	guide.ChangeColour("7", 255);
	guide.ChangeColour("16", 255);
	guide.PrintEntities();
}

#ifdef BAZEL_BUILD
#include "examples/protos/multiplayer_scene.grpc.pb.h"
#else
#include "multiplayer_scene.grpc.pb.h"
#endif
using multiplayerscene::Entity;

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
static int mainInit(GLFWwindow*& window, int width, int height)
{
	// TODO: Platform-specific get for window resolution, pass it into screen.
	glfwSetErrorCallback(glfw_error_callback);

	// true == 1, false == 0
	if (!glfwInit())
		return 1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); /* 4.5 is our latest version, reduce this for public release. */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 8); /* MSAA */

	// Create window with graphics context
	window = glfwCreateWindow(width, height, "Multiplayer Scene", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		return -1;
	}
	glEnable(GL_MULTISAMPLE); /* MSAA */
	return 0;
}

int main(int argc, char** argv) {
	GLFWwindow* window{ nullptr };
	mainInit(window, 600, 400);

	Scene scene{window};
	MultiplayerSceneClient client{ grpc::CreateChannel("localhost:50051",
						grpc::InsecureChannelCredentials())};


	glClearColor(0.5f, 0.5f, 0.7f, 1.0f);
	while (glfwGetKey(window,GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		Point userLocation{ vec3_to_point(scene.camera().m_position) };
		std::vector<Entity> entities{client.GetEntities(userLocation)};

		scene.updateEntities(entities);
		scene.draw_frame();

		glfwPollEvents();
		scene.process_keyboard();

		glfwSwapBuffers(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	return 0;
}
