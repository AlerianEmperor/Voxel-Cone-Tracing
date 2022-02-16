#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 ProjX;
uniform mat4 ProjY;
uniform mat4 ProjZ;

in Vertex
{
	vec2 TexCoord;
	vec4 DepthCoord;
} vertices[];

out Vertex_GS
{
	vec2 TexCoord;
	flat int axis;
	vec4 DepthCoord;
};

void main()
{
	vec3 e1 = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
	vec3 e2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;

	vec3 n = normalize(cross(e1, e2));

	n = vec3(abs(n.x), abs(n.y), abs(n.z));

	float nx = n.x, ny = n.y, nz = n.z;


	if(nx >= ny && nx >= nz)
		axis = 1;
	else if(ny >= nx && ny >= nz)
		axis = 2;
	else
		axis = 3;

	mat4 pMat = axis == 1 ? ProjX : axis == 2 ? ProjY : ProjZ;

	for(int i = 0; i < gl_in.length(); ++i)
	{
		TexCoord = vertices[i].TexCoord;
		DepthCoord = vertices[i].DepthCoord;
		gl_Position = pMat * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}