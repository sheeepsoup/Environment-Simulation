#version 450
layout(location = 0) out vec4 outColor;


layout(location = 0) in vec3 fragColor;//[x]->当前点的高度
layout(location = 1) in vec3 fragNormal;//法线
layout(location = 2) in vec3 fragWorldPosition;
layout(location = 3) in float fragFlow;
layout(location = 4) in vec4 fragCameraPos;
const vec3 dirFix = vec3(1,-1,0);//方向修饰,模拟现实特定方向风的风化
const vec3 worldUp = vec3(0,0,1);
const vec3 grassColor={ 0.18f, 0.38f, 0.14f};
const vec3 dirtColor={0.34f, 0.24f, 0.14f};
const vec3 rockColor={0.38f, 0.39f, 0.37f};
const vec3 snowColor={0.88f, 0.92f, 0.95f};
const vec3 fogColor = vec3(0.38, 0.62, 0.82);
const float TERRAIN_SCALE = 2.0f;

#define HASHSCALE1 .1031
#define HASHSCALE3 vec3(.1031, .1030, .0973)
#define HASHSCALE4 vec4(1031, .1030, .0973, .1099)
float Hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}
vec2 Hash22(vec2 p)
{
	vec3 p3 = fract(vec3(p.xyx) * HASHSCALE3);
    p3 += dot(p3, p3.yzx+19.19);
    return fract((p3.xx+p3.yz)*p3.zy);

}
vec2 add = vec2(1.0, 0.0);
float Noise(vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    
    float res = mix(mix( Hash12(p),          Hash12(p + add.xy),f.x),
                    mix( Hash12(p + add.yx), Hash12(p + add.xx),f.x),f.y);
    return res;
}

vec2 Noise2(vec2 x)
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y * 57.0;
   vec2 res = mix(mix( Hash22(p),          Hash22(p + add.xy),f.x),
                  mix( Hash22(p + add.yx), Hash22(p + add.xx),f.x),f.y);
    return res;
}
//--------------------------------------------------------------------------------------------------
//检测数字是否在界限内
bool numInEpsilon(float num,float leftEpsilon,float rightEpsilon){
    if(num <= rightEpsilon && num >= leftEpsilon)return true;
    else{return false;};
}
//获取纹理属性权重[1草][2泥土][3岩石][4雪地]
vec4 getTexureIndex(vec3 normal,vec3 pos){
    //获取坡度
    float pot = dot(normal,dirFix);
    float upness = clamp(dot(normal, worldUp), 0.0, 1.0);
    float slope = 1.0 - upness;
 
    float directionNoise = Noise(pos.xy * 0.015) * 2.0 - 1.0;//噪声扰乱
    pot = clamp(pot + directionNoise * 0.20,0.0,1.0);

    //噪声干扰边界
    float largeNoise = Noise(pos.xy * 0.025);
    float smallNoise = Noise(pos.xy * 0.12);
    float boundaryNoise = ((largeNoise * 0.75 + smallNoise * 0.25) * 2.0 - 1.0);
    float materialHeight = pos.z + boundaryNoise * 0.5;

	//草地权重,跟高度关系最大
	float grassNormalWeight = 1 - smoothstep(0.0f,0.35f, pot);//草原法线权重
	float grassHeightWeight = 1 - smoothstep(0.0f, 5.0f, materialHeight);//草原高度权重
	float grassWeight = mix(grassHeightWeight, grassNormalWeight, 0.5f);//草原权重
    float grassAltitudeFade =
    1.0 - smoothstep(6.0, 9.0, materialHeight);
    grassWeight *= grassAltitudeFade;

	//泥土权重
	float dirtNormalWeight = smoothstep(0.0f, 0.35f, pot);//泥土法线权重
	float dirtHeightWeight = smoothstep(3.0f, 6.0f, materialHeight);//泥土高度权重
	float dirtWeight =mix(dirtHeightWeight, dirtNormalWeight, 0.5f);//泥土权重
	float highAltitudeDirtFade = 1.0f - smoothstep(6.5f, 9.0f, materialHeight);
	dirtWeight *= highAltitudeDirtFade;//去掉泥土环

	//岩石权重
	float rockNormalWeight = smoothstep(0.0f, 0.85f, pot);//岩石法线权重
	float rockHeightWeight = smoothstep(6.0f, 9.0f, materialHeight);//岩石高度权重
    float mountainMask = smoothstep(5.0, 8.0, materialHeight);
	float rockWeight =
    mix(rockHeightWeight, rockNormalWeight, 0.4);//岩石权重
	
	//雪地权重
	float snowNormalWeight = smoothstep(0.55f, 0.90f, upness);//雪地法线权重
	float snowHeightWeight = smoothstep(12.0f, 15.0f, materialHeight);//雪地高度权重
	float baseSnow = snowHeightWeight;//雪地权重
   

    float snowChannel = smoothstep(0.05, 0.40, fragFlow);//流量影响的沟壑
    float moderateSlope = smoothstep(0.08, 0.22, slope);//去除平坦区域流量影响
    float channelHeightMask = smoothstep(7.5, 10.0, materialHeight);//沟壑高度限制区间
    float lowerChannelSnow = snowChannel * moderateSlope * channelHeightMask;//低处的雪沟壑
    float upperLargeNoise = Noise(pos.xy * 0.05 + vec2(31.7, 17.3));//俩噪声扰动
    float upperSmallNoise = Noise(pos.xy * 0.25 + vec2(11.9, 47.1));
    float upperNoise = (upperLargeNoise * 0.75 + upperSmallNoise * 0.25) * 2.0 - 1.0;//调整2合并-1~1
    float upperSnowHeight = materialHeight + upperNoise * 0.75 + lowerChannelSnow * 1.2;//噪声调整一波高度[影响效果]
    float upperBaseSnow = smoothstep(12.0, 15.0, upperSnowHeight);//高处的基础雪生成

    //合并低处雪沟和上方雪圈
    float snowCoverage = 1.0 - (1.0 - lowerChannelSnow) * (1.0 - upperBaseSnow);
    snowCoverage = clamp(snowCoverage, 0.0, 1.0);

    return vec4(grassWeight,dirtWeight,rockWeight,snowCoverage);
}
//获取纹理颜色
vec3 getTextureColor(vec4 weight){
    weight = max(weight, vec4(0.0));
    //根据侵蚀模拟给泥土颜色
    // 干燥土壤：偏浅、偏黄
    const vec3 dryDirtColor = vec3(0.42, 0.31, 0.18);

// 潮湿土壤：更深、更冷，避免直接变成纯黑
    const vec3 wetDirtColor = vec3(0.16, 0.12, 0.085);
    float wetness = smoothstep(0.08, 0.55, fragFlow);

    vec3 finalDirtColor = mix(
        dryDirtColor,
        wetDirtColor,
        wetness
    );
    vec3 terrainColor =
    grassColor * weight.x +
    finalDirtColor  * weight.y +
    rockColor  * weight.z +
    snowColor  * weight.w;

    return terrainColor;
}
void main(){
   
    vec3 normal = normalize(fragNormal);
    vec4 TexutreAttribute = getTexureIndex(normal,fragWorldPosition / TERRAIN_SCALE);//计算纹理
    vec4 colora = vec4(getTextureColor(TexutreAttribute), 1.0);

    //光照计算
    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDirection),0.0 );
    float ambient = 0.20;

    vec3 finalColor = colora.xyz *(ambient + diffuse * 0.80);



    //-----------------------------------------------------------------------计算雾
    float dist = length(fragCameraPos.xyz - fragWorldPosition);
    float fogValue = 1 - smoothstep(80.0f,450.0f,dist);
    finalColor = mix(fogColor,finalColor,fogValue);

    outColor = vec4(finalColor, 1.0);
   //outColor = vec4(vec3(fragFlow), 1.0);
   //outColor = vec4(TexutreAttribute.a);
}

