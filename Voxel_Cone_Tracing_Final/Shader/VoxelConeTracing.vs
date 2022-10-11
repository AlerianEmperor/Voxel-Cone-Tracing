#version 430 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;
layout (location = 3) in vec3 Tangent;
layout (location = 4) in vec3 BiTangent;

out vec2 tex;
out vec3 Position_world;
out vec3 Normal_world;
out vec3 Tangent_world;
out vec3 BiTangent_world;
out vec3 CameraDirection_world;
out vec4 Position_depth;

uniform vec3 CameraPosition;
uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 DepthModelViewProjectionMatrix;

void main()
{
	gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(Position, 1.0f);

	Position_world = (ModelMatrix * vec4(Position, 1.0f)).xyz;
	Position_depth = DepthModelViewProjectionMatrix * vec4(Position, 1.0f);
	Position_depth.xyz = Position_depth.xyz * 0.5f + vec3(0.5f);

	Normal_world = (ModelMatrix * vec4(Normal, 0.0f)).xyz;
	Tangent_world = (ModelMatrix * vec4(Tangent, 0.0f)).xyz;
	BiTangent_world = (ModelMatrix * vec4(BiTangent, 0.0f)).xyz;
	CameraDirection_world = CameraPosition - Position_world;

	tex = TexCoord;
}

