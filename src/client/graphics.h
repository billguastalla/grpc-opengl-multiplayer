// (c) 2022 Bill Guastalla, Apache 2.0 License.

#include <vector>

#include <visualisations/camera/Camera.h>
#include <visualisations/graphics/Shader.h>
#include <GLFW/glfw3.h>

#ifdef BAZEL_BUILD
#include "examples/protos/multiplayer_scene.grpc.pb.h"
#else
#include "multiplayer_scene.grpc.pb.h"
#endif

using multiplayerscene::Point;

inline Point vec3_to_point(const glm::vec3& v) {
	Point p; // NOTE: Can we just to a direct memory copy?
	p.set_x(v.x);
	p.set_y(v.y);
	p.set_z(v.z);
	return p;
}
inline glm::vec3 point_to_vec3(const Point& p) {
	return glm::vec3{ p.x(),p.y(),p.z() };
}
inline glm::vec3 colourIntToVec3(const int32_t col) {
	return glm::vec3{ ((col & 255u) / 256.),(((col & (255u << 8)) >> 8) / 256.),(((col & (255u << 16)) >> 16) / 256.) };
}

using multiplayerscene::Entity;
using multiplayerscene::User;

class Scene {
public:
	Scene(GLFWwindow* window, std::string uid) :
		m_generalShader
	{
		"../../shaders/Mesh_Vertex.vs",
		"../../shaders/Mesh_Fragment.fs"
	},
		m_uid{ uid },
		p_window{ window },
		m_camera{},
		m_entities{}
	{}

	void draw_frame();
	void updateEntities(std::vector<Entity> entities) { m_entities = entities; }
	void updateUsers(std::vector<User> users) { m_users = users; };


	User updateAndReturnThisUser() {
		auto foundUser{ std::find_if(m_users.begin(), m_users.end(), [this](const User& u) { return u.id() == m_uid; })};
		if (foundUser != m_users.end()) {
			foundUser->mutable_location()->set_x(m_camera.m_position.x);
			foundUser->mutable_location()->set_y(m_camera.m_position.y);
			foundUser->mutable_location()->set_z(m_camera.m_position.z);
			//Geometry::quat(m_camera.m_orientation);
			return *foundUser;
		}
		else
			return User{};
	}
	void process_keyboard();
	const Camera& camera() { return m_camera; }
private:
	Shader m_generalShader;

	GLFWwindow* p_window;
	Camera m_camera;
	std::string m_uid;

	std::vector<Entity> m_entities;
	std::vector<User> m_users;


	bool m_firstMouse{ false };
	int m_lastX{ 0 }, m_lastY{ 0 };
};
