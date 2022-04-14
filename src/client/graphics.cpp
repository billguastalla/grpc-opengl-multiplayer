#include "graphics.h"
#include <glad/glad.h>

#include <visualisations/graphics/Mesh.h>
#include <visualisations/graphics/MeshGenerator.h>

void Scene::draw_frame()
{
	Mesh m; // unnecessary to redo every frame.
	MeshGenerator::generateCube(m); // unnecessary to redo every frame.

	m_entityShader.use();
	m_entityShader.setVec3("lightColour", glm::vec3{ 0.9f,0.8f,0.81f }); // unnecessary to redo every frame.

																		 // pass projection matrix to shader (note that in this case it could change every frame)
	glm::mat4 projection{ m_camera.projectionMatrix() };
	// camera/view transformation
	glm::mat4 view = m_camera.GetViewMatrix();

	glm::mat4 m_mainModelMat{1.0};
	m_entityShader.setMat4("projection", projection);
	m_entityShader.setMat4("view", view);
	m_entityShader.setMat4("model", m_mainModelMat);

	glm::mat4 lightModel{ 1.0f };
	lightModel = glm::translate(lightModel, {0.,5.,0.});		// Note: Light is in the middle for 
	lightModel = glm::scale(lightModel, glm::vec3{ 0.2f });
	m_entityShader.setVec3("lightPos", { 0.,5.,0. });
	m_entityShader.setVec3("viewPos", m_camera.m_position);

	for (const auto & entity : m_entities) {
		Mesh entityMesh{ m };
		entityMesh.translate(point_to_vec3(entity.location()));

		m_entityShader.setVec3("objectColour", colourIntToVec3(entity.colour()));
		entityMesh.draw(&m_entityShader);

		entityMesh.clear();
	}
	m.clear();
}

void Scene::process_keyboard()
{
	/* Movement */
	int cm{ 0 };
	int w = glfwGetKey(p_window, GLFW_KEY_W);
	int a = glfwGetKey(p_window, GLFW_KEY_A);
	int s = glfwGetKey(p_window, GLFW_KEY_S);
	int d = glfwGetKey(p_window, GLFW_KEY_D);
	int space = glfwGetKey(p_window, GLFW_KEY_SPACE);
	int rightBracket = glfwGetKey(p_window, GLFW_KEY_RIGHT_BRACKET);
	int leftBracket = glfwGetKey(p_window, GLFW_KEY_LEFT_BRACKET);
	int zero = glfwGetKey(p_window, GLFW_KEY_0);
	if (w == GLFW_PRESS)
		cm += (int)Camera_Movement::FORWARD;
	if (a == GLFW_PRESS)
		cm += (int)Camera_Movement::LEFT;
	if (s == GLFW_PRESS)
		cm += (int)Camera_Movement::BACKWARD;
	if (d == GLFW_PRESS)
		cm += (int)Camera_Movement::RIGHT;
	if (space == GLFW_PRESS)
		cm += (int)Camera_Movement::UP;
	if (rightBracket == GLFW_PRESS)
		cm += (int)Camera_Movement::INCREASE_MOVEMENT_SPEED;
	if (leftBracket == GLFW_PRESS)
		cm += (int)Camera_Movement::DECREASE_MOVEMENT_SPEED;
	if (zero == GLFW_PRESS)
		cm += (int)Camera_Movement::RESET_POSITION;
	m_camera.ProcessKeyboard((Camera_Movement)cm, 0.01f);


	int leftMouse = glfwGetMouseButton(p_window, GLFW_MOUSE_BUTTON_1);
	double xPos{ 0.0 }, yPos{ 0.0 };
	glfwGetCursorPos(p_window, &xPos, &yPos);
	if (leftMouse == GLFW_PRESS)
	{
		if (m_firstMouse)
		{
			m_lastX = xPos;
			m_lastY = yPos;
			m_firstMouse = false;
		}
		else
		{
			float xoffset = xPos - m_lastX;
			float yoffset = m_lastY - yPos;
			m_lastX = xPos;
			m_lastY = yPos;
			m_camera.ProcessMouseMovement(xoffset, yoffset);
		}
	}
	else
		m_firstMouse = true;

}
