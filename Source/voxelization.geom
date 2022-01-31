#version 430 core
layout (triangles) in; 
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 ProjX;
uniform mat4 ProjY;
uniform mat4 ProjZ;

in VS_OUT {
    vec2 texCoord;
    vec4 depthCoord;
} vertices[];

out GS_OUT {
    vec2 texCoord;
    flat int axis;
    vec4 depthCoord;    //几何着色器不对深度纹理坐标做修改，直接传给片段着色器
};

void main() {
    /* 通过计算该三角形的两个向量，我们可以得到这个三角形的法向量 */
    vec3 p1 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;  //顶点坐标相减得向量
    vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(p1,p2));      //得法向量
    vec3 nn = vec3(abs(normal.x),abs(normal.y),abs(normal.z));
    float normalX=nn.x; float normalY=nn.y; float normalZ=nn.z;
    /* 选择有效面积最大的那个方向，确定最终要使用的projection矩阵 */
    if(normalX >= normalY && normalX >= normalZ)
        axis = 1;
    else if (normalY >= normalX && normalY >= normalZ)
        axis = 2;
    else
        axis = 3;
    mat4 projectionMatrix = axis == 1 ? ProjX : axis == 2 ? ProjY : ProjZ;
    /* 准备处理和发送每个顶点的数据 */
    for(int i = 0;i < gl_in.length(); i++) {
        texCoord = vertices[i].texCoord;
        depthCoord = vertices[i].depthCoord; //几何着色器不对深度纹理坐标做修改，直接传给片段着色器
        gl_Position = projectionMatrix * gl_in[i].gl_Position;  //乘以projection矩阵
        EmitVertex();
    }
    EndPrimitive();
}