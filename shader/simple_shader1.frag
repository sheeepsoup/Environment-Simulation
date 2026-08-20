#version 450


layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
void main(){
 vec3 normal = normalize(fragNormal);

    vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
    float diffuse = max(dot(normal, lightDirection),0.0 );
    float ambient = 0.20;
    vec3 finalColor = fragColor *(ambient + diffuse * 0.80);

    outColor = vec4(finalColor, 1.0);
}

