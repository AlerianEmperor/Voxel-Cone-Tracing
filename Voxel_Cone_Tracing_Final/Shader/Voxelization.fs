#version 430 core

uniform sampler2D DiffuseTexture;

uniform sampler2D ShadowMap;
uniform int ShadowMapSize;

uniform int VoxelDimensions;
uniform layout(RGBA8) image3D VoxelTexture;

in Vertex_GS
{
	vec2 TexCoord;
	flat int axis;
	vec4 DepthCoord;
} gs;

float PCF_Shadow_Mapping(float bias)
{
	//float current_depth = gl_FragCoord.z / gl_FragCoord.w;

	float current_depth = gs.DepthCoord.z / gs.DepthCoord.w;

	float shadow = 0.0f;

	int radius = 2;

	//float inv_shadow_size = 1.0f / ShadowMapSize;

	for(int x = -radius; x <= radius; ++x )
	{
		for(int y = -radius; y <= radius; ++y)
		{
			//vec2 offset = vec2(x * inv_shadow_size, y * inv_shadow_size);

			vec2 offset = vec2(1.0f / ShadowMapSize * x, 1.0f / ShadowMapSize * y);

			float closest_depth = texture(ShadowMap, vec2(gs.DepthCoord.xy + offset)).r;

			//use bias to avoid shadow acne
			if(current_depth - bias <= closest_depth)
				shadow += 1.0f;
		}
	}

	shadow /= (2 * radius + 1) * (2 * radius + 1);
	//shadow /= 9

	//shadow *= 0.111f;

	return shadow;
}

void main()
{
	vec4 color = texture(DiffuseTexture, gs.TexCoord);

	ivec3 camPos = ivec3(gl_FragCoord.x, gl_FragCoord.y, VoxelDimensions * gl_FragCoord.z);

	ivec3 voxelPos;

	//OpenGL insights page 306
	//use VoxelDimensions - camPos because the new original of projection space is at
	//vec3(VoxelDimesions)
	//minus 1 to 
	
	//
	//
	// 
	if(gs.axis == 1) 
	{
	    voxelPos.x = VoxelDimensions - 1 - camPos.z;
		voxelPos.z = VoxelDimensions - 1 - camPos.x;
		voxelPos.y = camPos.y;
	}
	else if(gs.axis == 2) 
	{
	    voxelPos.z = VoxelDimensions - 1 - camPos.y;
		voxelPos.y = VoxelDimensions - 1 - camPos.z;
		voxelPos.x = camPos.x;
	} 
	else 
	{
	    voxelPos = camPos;
		voxelPos.z = VoxelDimensions - 1 - camPos.z;
	}

	imageStore(VoxelTexture, voxelPos, vec4(color.rgb * PCF_Shadow_Mapping(0.002), 1.0f));
}
