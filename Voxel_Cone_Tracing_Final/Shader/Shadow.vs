#version 430 core

layout(location = 0) in vec3 position;

uniform mat4 DepthModelViewProjectionMatrix;

void main()
{
	gl_Position = DepthModelViewProjectionMatrix * vec4(position, 1.0f);
}
