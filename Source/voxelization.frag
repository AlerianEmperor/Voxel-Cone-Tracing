#version 430

uniform sampler2D DiffuseTexture;

uniform sampler2D ShadowMap;
uniform int shadowMapRes;

uniform int VoxelDimensions;
uniform layout(RGBA8) image3D VoxelTexture;


in GS_OUT {
    vec2 texCoord;
    flat int axis;
    vec4 depthCoord; 
} frag;

float countShadow(float bias)	
{
	float current=frag.depthCoord.z/frag.depthCoord.w;
	float shadow = 0.0f;
    int radius = 2;
	for(int x=-radius;x<=radius;x++)		
	{
		for(int y=-radius;y<=radius;y++)
		{
            vec2 offset = vec2(1.0/ shadowMapRes * x, 1.0/ shadowMapRes * y);
			float closest = texture(ShadowMap, vec2(frag.depthCoord.xy+offset)).r;
			if(current - bias > closest)
				shadow+=0.0f;
			else 
				shadow+=1.0f;
		}
	}
	shadow/=(2*radius+1)*(2*radius+1);
	return shadow;
}

void main() {
    vec4 materialColor = texture(DiffuseTexture, frag.texCoord);
	ivec3 camPos = ivec3(gl_FragCoord.x, gl_FragCoord.y, VoxelDimensions * gl_FragCoord.z);
	ivec3 voxelPos;
	if(frag.axis == 1) 
	{
	    voxelPos.x = VoxelDimensions -1 - camPos.z;
		voxelPos.z = VoxelDimensions -1 -camPos.x;
		voxelPos.y = camPos.y;
	}
	else if(frag.axis == 2) 
	{
	    voxelPos.z = VoxelDimensions -1 -camPos.y;
		voxelPos.y = VoxelDimensions -1 - camPos.z;
		voxelPos.x = camPos.x;
	} 
	else 
	{
	    voxelPos = camPos;
		voxelPos.z = VoxelDimensions -1 -camPos.z;
	}
    imageStore(VoxelTexture, voxelPos, vec4(materialColor.rgb * countShadow(0.002), 1.0));
}
