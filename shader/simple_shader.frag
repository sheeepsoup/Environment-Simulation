#version 450
layout(location = 0) out vec4 outColor;


layout(location = 0) in vec3 fragColor;//[x]->当前点的高度
layout(location = 1) in vec3 fragNormal;//法线
layout(location = 2) in vec3 fragWorldPosition;
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

void main(){
   
    vec3 normal = normalize(fragNormal);




    //光照计算
    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDirection),0.0 );
    float ambient = 0.20;

    //纹理计算


    // 混合两种土色（沙土 vs 腐殖土）
    vec3 soilColor = mix(
        vec3(0.6, 0.5, 0.3),  // 干燥沙土
        vec3(0.3, 0.2, 0.1),  // 湿润腐殖土
        0.5
    );


    // 输出
    vec4 colora = vec4(soilColor, 1.0);


    vec3 finalColor = colora.xyz *(ambient + diffuse * 0.80);

    outColor = vec4(finalColor, 1.0);


    outColor = vec4(Noise2(fragWorldPosition.xy).g);
}

