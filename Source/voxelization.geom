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
    vec4 depthCoord;    
};

void main() {
    
    vec3 p1 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;  
    vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 normal = normalize(cross(p1,p2));      
    vec3 nn = vec3(abs(normal.x),abs(normal.y),abs(normal.z));
    float normalX=nn.x; float normalY=nn.y; float normalZ=nn.z;
   
    if(normalX >= normalY && normalX >= normalZ)
        axis = 1;
    else if (normalY >= normalX && normalY >= normalZ)
        axis = 2;
    else
        axis = 3;
    mat4 projectionMatrix = axis == 1 ? ProjX : axis == 2 ? ProjY : ProjZ;
    
    for(int i = 0;i < gl_in.length(); i++) {
        texCoord = vertices[i].texCoord;
        depthCoord = vertices[i].depthCoord; 
        gl_Position = projectionMatrix * gl_in[i].gl_Position;  
        EmitVertex();
    }
    EndPrimitive();
}
