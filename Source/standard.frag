#version 430 core
//顶点性质
in vec2 UV;
in vec3 Position_world;
in vec3 Normal_world;
in vec3 Tangent_world;
in vec3 Bitangent_world;
in vec3 EyeDirection_world;
in vec4 Position_depth; 

// 纹理
uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D MaskTexture;
uniform sampler2D HeightTexture;
uniform vec2 HeightTextureSize;
uniform sampler2D ShadowMap;
uniform int shadowMapRes;
uniform vec3 LightDirection; //平行光源
uniform float ambientFactor;

// 材料性质
uniform float Shininess;
uniform float Opacity;

// 体素常量
uniform sampler3D VoxelTexture;
uniform float VoxelGridWorldSize;   //150.f
uniform int VoxelDimensions;    //512

// 显示开关
uniform float ShowDiffuse;
uniform float ShowIndirectDiffuse;
uniform float ShowDirectSpecular;
uniform float ShowIndirectSpecular;
uniform float AmbientOcclusion;

//最终输出
out vec4 color;

const float MAX_DIST = 75.0;  //沿锥中心线最大距离
const float ALPHA_THRESH = 0.95; 

//漫反射锥
const int NUM_CONES = 6; // 60度
float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);//各锥体权重
vec3 coneDirections[6] = vec3[]
                            ( 
                            vec3(0, 0, 1),
                            vec3(0, 0.866025,0.5),
                            vec3(0.823639, 0.267617, 0.5),
                            vec3(0.509037, -0.700629, 0.5),
                            vec3(-0.509037, -0.700629, 0.5),
                            vec3(-0.823639, 0.267617, 0.5)
                            );

vec4 sampleVoxels(vec3 worldPosition, float lod) {
    //求在体素空间中的坐标以及属性
    vec3 voxelTextureUV = worldPosition / (VoxelGridWorldSize * 0.5);  
    voxelTextureUV = voxelTextureUV * 0.5 + 0.5;
    return textureLod(VoxelTexture, voxelTextureUV, lod);
}

    //沿锥累积
vec4 coneTrace(vec3 direction, float tanHalfAngle) {
    //累积变量初始化
    float lod = 0.0; //mipmap级别
    vec3 color = vec3(0);
    float alpha = 0.0;
    float occlusion = 0.0;

    float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions;  //单个体素的边长
    float dist = voxelWorldSize; // 避免自遮蔽
    vec3 startPos = Position_world + Normal_world * voxelWorldSize; //避免在平坦表面形成自遮蔽

    while(dist < MAX_DIST && alpha < ALPHA_THRESH ) {
        // 最小取样直径（三角形的底与体素大小间取大值）
        float diameter = max(voxelWorldSize, 2.0 * tanHalfAngle * dist);
        float lodLevel = log2(diameter / voxelWorldSize); //使用对数计算取样级别
        vec4 voxelColor = sampleVoxels(startPos + dist * direction, lodLevel);//取对应体素的颜色

        // 沿锥体中心线方向叠加
        color +=  (1.0 - alpha) * voxelColor.rgb;
        occlusion += ( (1.0 - alpha) * voxelColor.a) / (1.0 + 0.03 * diameter);
        alpha +=  (1.0 - alpha) * voxelColor.a;
        dist += diameter; 
    }

    return vec4(color, occlusion);
}

//由高度图计算法线
mat3 TBN;//切线空间变换到世界空间
vec3 calcBumpNormal() {
    // 计算x,y轴梯度
    vec2 offset = vec2(1.0) / HeightTextureSize;
    float curr = texture(HeightTexture, UV).r;
    float diffX = texture(HeightTexture, UV + vec2(offset.x, 0.0)).r - curr;
    float diffY = texture(HeightTexture, UV + vec2(0.0, offset.y)).r - curr;
    //叉乘计算法线
    vec3 t1 = normalize(vec3(1,0,diffX));
    vec3 t2 = normalize(vec3(0,1,diffY));
    vec3 bumpNormal = normalize(cross(t1,t2));
    //映射到世界空间
    return normalize(TBN * bumpNormal);
}

float countShadow(float bias)	//bias是偏移值，防止阴影走样
{
	float current=Position_depth.z/Position_depth.w;
	float shadow = 0.0f;
    int radius = 2;
	for(int x=-radius;x<=radius;x++)		//这里用的是PCF,采样空间3x3
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
    vec4 materialColor = texture(DiffuseTexture, UV);//材料纹理颜色
    float alpha = materialColor.a;//材料透明度
    float occlusion = 0.0;
    float specularOcclusion = 0.0;
    if(alpha < 0.5) {
        discard;
    }//不渲染透明物体
    
    //TBN矩阵
    TBN = inverse(transpose(mat3(Tangent_world,Bitangent_world, Normal_world)));

    vec3 N = calcBumpNormal();//从高度贴图计算法向量
    vec3 L = normalize(LightDirection); //光源方向
    vec3 E = normalize(EyeDirection_world);//相机相对方向
   
       // 计算可见度
       float visibility = countShadow(0.002);

//计算漫反射光
    vec3 diffuseReflection;
    {

        // 直接漫反射
        float cosTheta = max(0, dot(N, L));//入射光线和法线余弦值
        vec3 directDiffuseLight = ShowDiffuse > 0.5 ? vec3(visibility * cosTheta) : vec3(0.0);//是否让直接漫反射光可见

        //间接漫反射
       vec4 indirectDiffuseLight = vec4(0.f);
        for(int i = 0; i < NUM_CONES; i++) //叠加六个锥体
        {
            indirectDiffuseLight += coneWeights[i] * coneTrace(normalize(TBN * coneDirections[i]), 0.577);
        }
        occlusion = 1 - indirectDiffuseLight.a;
        indirectDiffuseLight = ShowIndirectDiffuse > 0.5 ?  indirectDiffuseLight :vec4(0.0);

        // 漫反射求和
        diffuseReflection =  (directDiffuseLight +    occlusion * indirectDiffuseLight.rgb ) * materialColor.rgb;
    }
    
    // 计算镜面反射光
    vec3 specularReflection;
    {
        vec4 specularColor = texture(SpecularTexture, UV);
        specularColor = length(specularColor.gb) > 0.0 ? specularColor : specularColor.rrra;

        //直接镜面反射
        vec3 reflectDir = normalize(reflect(-L, N));  
        float spec = pow(max(dot(E, reflectDir), 0.0),Shininess);
        vec3 directSpecularLight = vec3 (spec * visibility); 
        directSpecularLight = ShowDirectSpecular > 0.5 ? directSpecularLight : vec3(0.0);
       
       //间接镜面反射
        vec3 inlightDir = normalize(reflect(-E,N));
        vec4 tracedSpecular = coneTrace(inlightDir, tan(0.3)); //  0.07 = 8 degrees angle
        tracedSpecular = ShowIndirectSpecular > 0.5 ?  tracedSpecular : vec4(0.0);
        specularOcclusion = 1 - tracedSpecular.a;
        //镜面反射求和
        specularReflection =   (tracedSpecular.rgb+ specularOcclusion * directSpecularLight) * specularColor.rgb ;
       
    }
    //环境光
    vec3 ambientLight =AmbientOcclusion>0.5? ambientFactor * materialColor.rgb* occlusion :ambientFactor * materialColor.rgb ;
    //光总量求和
    color = vec4(ambientLight + diffuseReflection + specularReflection, alpha);

}