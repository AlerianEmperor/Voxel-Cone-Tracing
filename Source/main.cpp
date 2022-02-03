
#include "Render.h"

#include <stdio.h>
#include <stdlib.h>

//#include <gl\glfw3.h>
//#include <glm/glm.hpp>

#include <iostream>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int shadowMapRes = 4096;
const float moveSpeed = 5.0f;

// camera
Camera camera(glm::vec3(0.0f, 4.0f, 0.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

bool PRESS_SPACE = false;
bool mouseCursor = false;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void dumpGLInfo() {
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void dumpGLErrors() {
	GLenum glError = glGetError();
	if (glError) {
		switch (glError) {
		case GL_INVALID_ENUM:
			std::cout << "Invalid enum." << std::endl;
			break;

		case GL_INVALID_VALUE:
			std::cout << "Invalid value." << std::endl;
			break;

		case GL_INVALID_OPERATION:
			std::cout << "Invalid operation." << std::endl;

		default:
			std::cout << "Unrecognised GLenum." << std::endl;
			break;
		}
	}
}

int main(void) {
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return EXIT_FAILURE;
	}

	glewExperimental = true;
	//glewInit();

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VCT", NULL, NULL);

	if (window == NULL) {
		fprintf(stderr, "Failed to Create OpenGL Context");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/*if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}*/

	//dumpGLInfo();
	//dumpGLErrors();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	double previousTime, currentTime;
	previousTime = glfwGetTime();

	camera.MovementSpeed = moveSpeed;

	Renderer renderer(SCR_WIDTH, SCR_HEIGHT, window);
	renderer.initialize(&camera, shadowMapRes);

	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		renderer.update(deltaTime);

		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return EXIT_SUCCESS;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !PRESS_SPACE)
	{
		PRESS_SPACE = true;
		mouseCursor = !mouseCursor;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
		PRESS_SPACE = false;
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, mouseCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;
	if (mouseCursor)
		camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
