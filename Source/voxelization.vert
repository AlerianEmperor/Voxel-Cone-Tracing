#version 430 core

layout(location = 0) in vec3 aPosition;  //模型的世界坐标
layout(location = 2) in vec2 vertexCoord;           //对应的纹理坐标

uniform mat4 DepthModelViewProjectionMatrix;
uniform mat4 ModelMatrix;

out VS_OUT {
    vec2 texCoord;
    vec4 depthCoord;    //与当前顶点对应的深度纹理上的坐标
} ;

void main() {
    /* 这里只乘了model矩阵而没有乘projection矩阵，我们会在几何着色器里确定用哪个方向的projection矩阵，到时候再乘 */
    gl_Position = ModelMatrix * vec4(aPosition,1);
    texCoord = vertexCoord;
    depthCoord = DepthModelViewProjectionMatrix * vec4(aPosition, 1);   
	depthCoord.xyz = depthCoord.xyz * 0.5f + 0.5f;    //把xyz变换到[0,1]
}