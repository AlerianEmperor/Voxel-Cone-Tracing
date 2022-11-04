#version 430 core

in vec2 tex;//
in vec3 Position_world;//
in vec3 Normal_world;//
in vec3 Tangent_world;//
in vec3 BiTangent_world;//
in vec3 CameraDirection_world;//
in vec4 Position_depth;//

//Textures Sample
uniform sampler2D DiffuseTexture;//
uniform sampler2D SpecularTexture;//
uniform sampler2D MaskTexture;//
uniform sampler2D HeightTexture;//
uniform vec2 HeightTextureSize;//

//Material Properties
uniform float Shininess;//
uniform float Opacity;//

//Light
uniform vec3 LightDirection;//
uniform float ambientFactor; //

//ShadowMap
uniform sampler2D ShadowMap;//
uniform int ShadowMapSize;//

//Voxel Properties
uniform sampler3D VoxelTexture;//
uniform float VoxelGridWorldSize; // 150.0f
uniform int VoxelDimensions;      // 512

//Adjust Scene Display
uniform float ShowDiffuse;
uniform float ShowIndirectDiffuse;
uniform float ShowSpecular;
uniform float ShowIndirectSpecular;

out vec4 color;

const float MAX_DISTANCE = 495.0;
const float MAX_ALPHA = 1.0;

const int NUM_CONES = 6;

const float pi = 3.1415926f;

float Cone_Weights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);
vec3 Cone_Directions[6] = vec3[]
								( 
								vec3(0, 0, 1),
								vec3(0, 0.866025,0.5),
								vec3(0.823639, 0.267617, 0.5),
								vec3(0.509037, -0.700629, 0.5),
								vec3(-0.509037, -0.700629, 0.5),
								vec3(-0.823639, 0.267617, 0.5)
								);

vec4 SampleVoxels(vec3 World_Position, float lod)
{
	vec3 Voxel_Texture_UV = World_Position / (VoxelGridWorldSize * 0.5f);

	Voxel_Texture_UV = Voxel_Texture_UV * 0.5f + 0.5f;//vec3(0.5f);
	
	return textureLod(VoxelTexture, Voxel_Texture_UV, lod);
}

/*vec3 hash( uvec3 x )
{
    x = ((x>>8U)^x.yzx)*k;
    x = ((x>>8U)^x.yzx)*k;
    x = ((x>>8U)^x.yzx)*k;

    return vec3(x)*(1.0/float(0xffffffffU));
}*/

//Instead of Marching by Sphere in "SPHERE TRACING" or by a constant Distance in "RAY MARCHING"
//we march by Voxels increase in size a long a cone, hence the name "Voxel Cone Tracing"
//when Voxel Size increase, we also increase the level of mipmap if 3D Texture sample
//this can be resemble by building a Voxel Octree to reduce meory overhead


//Voxel Cone Tracing Compute Radiance Value along a "Direction" with "Cone Angle Width" = "Angle"
//tanHalfAngle vary between different Material

//However we will fix this value to be constant among all specular Material

//Small tanHalfAngle lead to shiny Material
//Big tanHalfAngle lead to Blurry Material
vec4 Voxel_Cone_Tracing(vec3 direction, float tanHalfAngle)
{
	float lod = 0.0f;
	vec3 color = vec3(0.0f);
	float alpha = 0.0f;
	float occlusion = 0.0f;

	//Voxel Cube Size
	float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions;
	float dist = voxelWorldSize;
	vec3 startPos = Position_world + Normal_world * voxelWorldSize;

	while(dist < MAX_DISTANCE && alpha < MAX_ALPHA)
	{
		float diameter = max(voxelWorldSize, 2.0f * tanHalfAngle * dist);
		float lodLevel = log2(diameter / voxelWorldSize);
		vec4 voxelColor = SampleVoxels(startPos + dist * direction, lodLevel);

		color += (1.0f - alpha) * voxelColor.rgb;
		occlusion += ((1.0f - alpha) * voxelColor.a) / (1.0f + 0.03f * diameter);
		alpha += (1.0f - alpha) * voxelColor.a;
		dist += diameter;
	}

	return vec4(color, occlusion);
}


vec3 CalcBumpNormal(mat3 TBN)
{
	vec2 offset = vec2(1.0) / HeightTextureSize;

	float current = texture(HeightTexture, tex).r;
	float dx = texture(HeightTexture, tex + vec2(offset.x, 0.0f)).r - current;
	float dy = texture(HeightTexture, tex + vec2(0.0f, offset.y)).r - current;

	//float bump_strength = 2.0f;

	vec3 t1 = normalize(vec3(1.0f, 0.0f, dx));
	vec3 t2 = normalize(vec3(0.0f, 1.0f, dy));

	//t1 x t2 = (dx, dy, 1.0f);

	vec3 bump_normal = normalize(cross(t1, t2));
	
	return normalize(TBN * bump_normal); 
}



float PCF_Shadow_Mapping(float bias)
{
	float current_Depth = Position_depth.z / Position_depth.w;
	float shadow = 0.0f;

	int radius = 2;

	//float inv_shadow_size = 1.0f / ShadowMapSize;

	for(int x = -radius; x <= radius; ++x)
	{
		for(int y = -radius; y <= radius; ++y)
		{
			//vec2 offset = vec2(inv_shadow_size * x, inv_shadow_size * y);

			vec2 offset = vec2(1.0f / ShadowMapSize * x, 1.0f / ShadowMapSize * y);

			float closest_depth = texture(ShadowMap, vec2(Position_depth.xy + offset)).r;

			if(current_Depth - bias <= closest_depth)
				shadow += 1.0f;
		}
	}

	//shadow /= 9
	//for faster computation
	shadow *= 0.111f;
	
	//shadow /= (2 * radius + 1) * (2 * radius + 1);

	return shadow;
}

float randf(float xx)
{         
    float x0=floor(xx);
    float x1=x0+1;
    float v0 = fract(sin (x0*.014686)*31718.927+x0);
    float v1 = fract(sin (x1*.014686)*31718.927+x1);          

    return (v0*(1-fract(xx))+v1*(fract(xx)))*2-1*sin(xx);
}

void main()
{
	vec4 matColor = texture(DiffuseTexture, tex);

	float alpha = matColor.a;

	if(alpha < 0.5f)
		discard;


	mat3 TBN = inverse(transpose(mat3(Tangent_world, BiTangent_world, Normal_world)));

	vec3 N = CalcBumpNormal(TBN);

	//blur sun

	/*uvec3 pos_u = (uvec3)Position_world;

	vec3 x1 = hash(pos_u);

	uvec3 pos2 = Position_world + uvec3(2);

	vec3 x2 = hash(pos2);

	float u1 = x1.x;
	float u2 = x2.x;*/

	float u1 = randf(0.5);
	float u2 = randf(0.5);

	float theta = 2 * pi * u1;
    float phi = acos(1 - 2 * u2);
    float x = sin(phi) * cos(theta);
    float y = sin(phi) * sin(theta);
    float z = cos(phi);

	vec3 light_center = Position_world + LightDirection;

	light_center += vec3(x, y, z);

	vec3 Blur_Light_Direction = (light_center - Position_world);

	//end_blur sun
	//vec3 L = normalize(LightDirection);

	vec3 L = Blur_Light_Direction;

	vec3 E = normalize(CameraDirection_world);


	//DIFFUSE computation

	float shadow_value = PCF_Shadow_Mapping(0.002f);
	 
	float cos_theta = max(dot(N, L), 0.0f);

	//vec3 directDiffuse = ShowDiffuse > 0.5f ? vec3(shadow_value * cos_theta) : vec3(0.0f);

	vec3 directDiffuse = vec3(shadow_value * cos_theta);

	vec4 inDirectDiffuse = vec4(0.0f);

	for(int i = 0; i < NUM_CONES; ++i)
	{
		inDirectDiffuse += Cone_Weights[i] * Voxel_Cone_Tracing(normalize(TBN * Cone_Directions[i]), 0.577);//tan(60 / 2)
	}

	float occlusion = 1.0f - inDirectDiffuse.a;

	//inDirectDiffuse = ShowIndirectDiffuse > 0.5f ? inDirectDiffuse

	vec3 DiffuseReflection = vec3(3, 2, 1) * (directDiffuse + occlusion * inDirectDiffuse.rgb) * matColor.rgb;


	//SPECULAR computation
	vec4 specColor = texture(SpecularTexture, tex);
	specColor = length(specColor.gb) > 0.0f ? specColor : specColor.rrra;

	vec3 SpecularReflect = normalize(reflect(-L, N));
	float spec = pow(max(dot(E, SpecularReflect), 0.0f), Shininess);
	vec3 directSpecular = vec3(spec * shadow_value);
	//directSpecular = ShowSpecular > 0.5f ? directSpecular : vec3(0.0f)

	vec3 inDirectReflect = normalize(reflect(-E, N));
	vec4 inDirectSpecular = Voxel_Cone_Tracing(inDirectReflect, 0.07f);//0.105);//reflect cone width = 12 degree
	                                                                   //half cone = 6 dgree
																	   //tan(6) = 0.105
	float specularOcclusion = 1.0f - inDirectSpecular.a;

	vec3 specularReflection = vec3(3, 2, 1) * (inDirectSpecular.rgb + specularOcclusion * directSpecular.rgb) * specColor.rgb;

	vec3 AmbientLight = ambientFactor * matColor.rgb * occlusion;

	color = vec4(AmbientLight + DiffuseReflection + specularReflection, alpha);
	
}
