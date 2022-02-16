#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Shader.h"

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

using namespace glm;

enum Camera_Direction
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float YAW = -90;
const float PITCH = 0.0f;
const float SPEED = 2.6f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0F;

//Taken from LearnOpenGL Camera Class
class Camera
{
public:
	//Camera Position
	vec3 position = vec3(0.0f, 10.0f, 0.0f);
	vec3 Front = vec3(0.0f, 0.0f, -1.0f);
	vec3 Up = vec3(0.0f, 1.0f, 0.0f);
	vec3 Right;
	vec3 WorldUp;

	//Rotation Angles
	//YAW and PITCH may cause confusion
	float Yaw = 0.0f;//Rotate Y
	float Pitch = 0.0f;//Rotate X
	//float Roll//Rotate Z, only use for air plane simulation

	//Camera Speed
	float MovementSpeed = 5.0f;
	float MouseSensitivity = 0.5f;
	float Zoom = 45.0f;

	
	Camera(vec3 position_ = vec3(0.0f, 0.0f, 0.0f), vec3 Up_ = vec3(0.0f, 1.0f, 0.0f), float Yaw_ = YAW, float Pitch_ = PITCH) : Yaw(Yaw_), Pitch(Pitch_)
	{
		position = position_;
		Front = vec3(0.0f, 0.0f, -1.0f);
		WorldUp = Up_;

		MovementSpeed = SPEED;
		MouseSensitivity = SENSITIVITY;

		UpdateCamera();
	}

	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float Yaw_ = YAW, float Pitch_ = PITCH) : Yaw(Yaw_), Pitch(Pitch_)
	{
		position = vec3(posX, posY, posZ);
		
		Front = vec3(0.0f, 0.0f, -1.0f);
		WorldUp = vec3(upX, upY, upZ);

		MovementSpeed = SPEED;
		MouseSensitivity = SENSITIVITY;

		UpdateCamera();
	}

	mat4 GetViewMatrix()
	{
		return lookAt(position, position + Front, Up);
	}

	void ProcessKeyBoard(Camera_Direction direction, float& deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;

		if (direction == FORWARD)
			position += Front * velocity;

		if (direction == BACKWARD)
			position -= Front * velocity;

		if (direction == LEFT)
			position -= Right * velocity;

		if (direction == RIGHT)
			position += Right * velocity;

		if (direction == UP)
			position += WorldUp * velocity;

		if (direction == DOWN)
			position -= WorldUp * velocity;
	}

	void ProcessMouseMovement(float& xOffset, float& yOffset, bool constrainPitch = true)
	{
		xOffset *= MouseSensitivity;
		yOffset *= MouseSensitivity;

		Yaw += xOffset;
		Pitch += yOffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}
		UpdateCamera();
	}

	void ProcessMouseScroll(float yOffset)
	{
		Zoom -= (float)yOffset;

		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

	void UpdateCamera()
	{
		vec3 front;

		front.x = cosf(glm::radians(Yaw)) * cosf(glm::radians(Pitch));
		front.y = sinf(glm::radians(Pitch));
		front.z = sinf(glm::radians(Yaw)) * cosf(glm::radians(Pitch));

		Front = normalize(front);

		Right = normalize(cross(Front, WorldUp));
		Up = normalize(cross(Right, Front));

	}
};

#endif // !_CAMERA_H_

