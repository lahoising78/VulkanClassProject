#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in float time;

layout(location = 0) out vec4 outColor;

// float rand(vec2 co)
// {
//     return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
// }

float rand(float x)
{
    return fract(sin(x) * 43758.5453);
}

void main()
{
    vec3 lightVector = vec3(0,0,1);
    float cosTheta = dot( fragNormal,lightVector );
    vec4 baseColor = texture(texSampler, fragTexCoord);

    baseColor += baseColor * cosTheta;
    // outColor.w = baseColor.w;

    // vec4 mCol = mix(baseColor, baseColor * 0.5, hatching);
    // vec4 mCol = mix(vec4(1,0.9,0.8,1), vec4(1,0.9,0.8,1) * 0.5, hatching);

    outColor = (baseColor + baseColor * cosTheta);
}
