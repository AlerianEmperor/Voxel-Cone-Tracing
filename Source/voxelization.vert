#version 430 core

layout(location = 0) in vec3 aPosition;  //ģ�͵���������
layout(location = 2) in vec2 vertexCoord;           //��Ӧ����������

uniform mat4 DepthModelViewProjectionMatrix;
uniform mat4 ModelMatrix;

out VS_OUT {
    vec2 texCoord;
    vec4 depthCoord;    //�뵱ǰ�����Ӧ����������ϵ�����
} ;

void main() {
    /* ����ֻ����model�����û�г�projection�������ǻ��ڼ�����ɫ����ȷ�����ĸ������projection���󣬵�ʱ���ٳ� */
    gl_Position = ModelMatrix * vec4(aPosition,1);
    texCoord = vertexCoord;
    depthCoord = DepthModelViewProjectionMatrix * vec4(aPosition, 1);   
	depthCoord.xyz = depthCoord.xyz * 0.5f + 0.5f;    //��xyz�任��[0,1]
}