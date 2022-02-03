#version 430 core

in vec2 UV;
in vec3 Position_world;
in vec3 Normal_world;
in vec3 Tangent_world;
in vec3 Bitangent_world;
in vec3 EyeDirection_world;
in vec4 Position_depth; 


uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D MaskTexture;
uniform sampler2D HeightTexture;
uniform vec2 HeightTextureSize;
uniform sampler2D ShadowMap;
uniform int shadowMapRes;
uniform vec3 LightDirection; 
uniform float ambientFactor;


uniform float Shininess;
uniform float Opacity;


uniform sampler3D VoxelTexture;
uniform float VoxelGridWorldSize;   //150.f
uniform int VoxelDimensions;    //512


uniform float ShowDiffuse;
uniform float ShowIndirectDiffuse;
uniform float ShowDirectSpecular;
uniform float ShowIndirectSpecular;
uniform float AmbientOcclusion;


out vec4 color;

const float MAX_DIST = 75.0;  
const float ALPHA_THRESH = 0.95; 


const int NUM_CONES = 6; 
float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);
vec3 coneDirections[6] = vec3[]
                            ( 
                            vec3(0, 0, 1),
                            vec3(0, 0.866025,0.5),
                            vec3(0.823639, 0.267617, 0.5),
                            vec3(0.509037, -0.700629, 0.5),
                            vec3(-0.509037, -0.700629, 0.5),
                            vec3(-0.823639, 0.267617, 0.5)
                            );

vec4 sampleVoxels(vec3 worldPosition, float lod)
{
    vec3 voxelTextureUV = worldPosition / (VoxelGridWorldSize * 0.5);  
    voxelTextureUV = voxelTextureUV * 0.5 + 0.5;
    return textureLod(VoxelTexture, voxelTextureUV, lod);
}

   
vec4 coneTrace(vec3 direction, float tanHalfAngle)
{
    float lod = 0.0; 
    vec3 color = vec3(0);
    float alpha = 0.0;
    float occlusion = 0.0;

    float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions; 
    float dist = voxelWorldSize; 
    vec3 startPos = Position_world + Normal_world * voxelWorldSize; 

    while(dist < MAX_DIST && alpha < ALPHA_THRESH ) {
     
        float diameter = max(voxelWorldSize, 2.0 * tanHalfAngle * dist);
        float lodLevel = log2(diameter / voxelWorldSize); 
        vec4 voxelColor = sampleVoxels(startPos + dist * direction, lodLevel);

       
        color +=  (1.0 - alpha) * voxelColor.rgb;
        occlusion += ( (1.0 - alpha) * voxelColor.a) / (1.0 + 0.03 * diameter);
        alpha +=  (1.0 - alpha) * voxelColor.a;
        dist += diameter; 
    }

    return vec4(color, occlusion);
}


mat3 TBN;
vec3 calcBumpNormal() {
    
    vec2 offset = vec2(1.0) / HeightTextureSize;
    float curr = texture(HeightTexture, UV).r;
    float diffX = texture(HeightTexture, UV + vec2(offset.x, 0.0)).r - curr;
    float diffY = texture(HeightTexture, UV + vec2(0.0, offset.y)).r - curr;
   
    vec3 t1 = normalize(vec3(1,0,diffX));
    vec3 t2 = normalize(vec3(0,1,diffY));
    vec3 bumpNormal = normalize(cross(t1,t2));
   
    return normalize(TBN * bumpNormal);
}

float countShadow(float bias)	
{
	float current=Position_depth.z/Position_depth.w;
	float shadow = 0.0f;
    int radius = 2;
	for(int x=-radius;x<=radius;x++)		
	{
		for(int y=-radius;y<=radius;y++)
		{
            vec2 offset = vec2(1.0/shadowMapRes * x, 1.0/shadowMapRes * y);
			float closest = texture(ShadowMap, vec2(Position_depth.xy+offset)).r;
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
    vec4 materialColor = texture(DiffuseTexture, UV);
    float alpha = materialColor.a;
    float occlusion = 0.0;
    float specularOcclusion = 0.0;
    if(alpha < 0.5) {
        discard;
    }
    
    TBN = inverse(transpose(mat3(Tangent_world,Bitangent_world, Normal_world)));

    vec3 N = calcBumpNormal();
    vec3 L = normalize(LightDirection); 
    vec3 E = normalize(EyeDirection_world);
         
       float visibility = countShadow(0.002);

    vec3 diffuseReflection;
    {      
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = ShowDiffuse > 0.5 ? vec3(visibility * cosTheta) : vec3(0.0);
     
       vec4 indirectDiffuseLight = vec4(0.f);
        for(int i = 0; i < NUM_CONES; i++) 
        {
            indirectDiffuseLight += coneWeights[i] * coneTrace(normalize(TBN * coneDirections[i]), 0.577);
        }
        occlusion = 1 - indirectDiffuseLight.a;
        indirectDiffuseLight = ShowIndirectDiffuse > 0.5 ?  indirectDiffuseLight :vec4(0.0);

       
        diffuseReflection =  (directDiffuseLight +    occlusion * indirectDiffuseLight.rgb ) * materialColor.rgb;
    }
    
    vec3 specularReflection;
    {
        vec4 specularColor = texture(SpecularTexture, UV);
        specularColor = length(specularColor.gb) > 0.0 ? specularColor : specularColor.rrra;

       
        vec3 reflectDir = normalize(reflect(-L, N));  
        float spec = pow(max(dot(E, reflectDir), 0.0),Shininess);
        vec3 directSpecularLight = vec3 (spec * visibility); 
        directSpecularLight = ShowDirectSpecular > 0.5 ? directSpecularLight : vec3(0.0);
       
      
        vec3 inlightDir = normalize(reflect(-E,N));
        vec4 tracedSpecular = coneTrace(inlightDir, tan(0.3)); //  0.07 = 8 degrees angle
        tracedSpecular = ShowIndirectSpecular > 0.5 ?  tracedSpecular : vec4(0.0);
        specularOcclusion = 1 - tracedSpecular.a;
       
        specularReflection =   (tracedSpecular.rgb+ specularOcclusion * directSpecularLight) * specularColor.rgb ;
       
    }
   
    vec3 ambientLight =AmbientOcclusion>0.5? ambientFactor * materialColor.rgb* occlusion :ambientFactor * materialColor.rgb ;
   
    color = vec4(ambientLight + diffuseReflection + specularReflection, alpha);

}
