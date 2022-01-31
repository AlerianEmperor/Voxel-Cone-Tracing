#version 430 core

layout(location = 0) in vec3 vertexPosition_model;

uniform mat4 ModelViewProjectionMatrix;

void main() {
	gl_Position = ModelViewProjectionMatrix * vec4(vertexPosition_model, 1);	//�ӹ�Դ�Ƕȱ任һ��
}