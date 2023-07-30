#include "Voxel_Cone_Tracing.h"

#include <stdio.h>
#include <stdlib.h>


bool firstClick = true;
bool mouseCursor = true;

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

float lastX = (float)(SCREEN_WIDTH / 2);
float lastY = (float)(SCREEN_HEIGHT / 2);

float delta_time = 0.0f;

void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void processInput(GLFWwindow* window);

void main()
{
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glewExperimental = GL_TRUE;

	//No GLFW_Samples cause program to flicker
	glfwWindowHint(GLFW_SAMPLES, 4);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Voxel Cone Tracing", NULL, NULL);

	if (window == NULL)
	{
		fprintf(stderr, "Failed to Create OpenGL Context");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
	{
		exit(EXIT_FAILURE);
	}

	//resize screen
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	float previous_time, current_time;

	previous_time = glfwGetTime();

	camera.MovementSpeed = 5.0f;
	camera.MouseSensitivity = 0.5f;
	Voxel_Cone_Tracing voxel_cone_tracing(SCREEN_WIDTH, SCREEN_HEIGHT, window);// , &camera);
	
	//voxel_cone_tracing.camera = &camera;
	//voxel_cone_tracing.window = window;

	float last_Time = 0.0f;

	

	while (!glfwWindowShouldClose(window))
	{
		

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float current_Time = glfwGetTime();

		delta_time = current_Time - last_Time;

		last_Time = current_Time;

		processInput(window);
		voxel_cone_tracing.Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	exit(EXIT_SUCCESS);
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstClick)
	{
		lastX = xPos;
		lastY = yPos;
		firstClick = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	//if (mouseCursor)
	camera.ProcessMouseMovement(xOffset, yOffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
	camera.ProcessMouseScroll(yOffset);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyBoard(FORWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyBoard(BACKWARD, delta_time);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyBoard(LEFT, delta_time);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyBoard(RIGHT, delta_time);

	//if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	//	mouseCursor = !mouseCursor;

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//mouseCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

