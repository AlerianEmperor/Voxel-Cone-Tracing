#version 430 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 tex;

uniform mat4 DepthModelViewProjectionMatrix;
uniform mat4 ModelMatrix;

out Vertex
{
	vec2 TexCoord;
	vec4 DepthCoord;
};

void main()
{
	TexCoord = tex;
	DepthCoord = DepthModelViewProjectionMatrix * vec4(position, 1.0f);
	DepthCoord.xyz = DepthCoord.xyz * 0.5f + vec3(0.5f);

	gl_Position = ModelMatrix * vec4(position, 1.0f);
}




