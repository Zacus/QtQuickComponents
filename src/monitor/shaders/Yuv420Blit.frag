#version 440

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform Yuv420Uniforms
{
    mat4 yuvToRgb;
    vec4 opacity;
} ubuf;

layout(binding = 1) uniform sampler2D yPlane;
layout(binding = 2) uniform sampler2D uPlane;
layout(binding = 3) uniform sampler2D vPlane;

void main()
{
    float y = texture(yPlane, vTexCoord).r;
    float u = texture(uPlane, vTexCoord).r;
    float v = texture(vPlane, vTexCoord).r;
    vec4 yuv = vec4(y, u, v, 1.0);
    vec3 rgb = vec3(
        dot(ubuf.yuvToRgb[0], yuv),
        dot(ubuf.yuvToRgb[1], yuv),
        dot(ubuf.yuvToRgb[2], yuv));
    fragColor = vec4(clamp(rgb, 0.0, 1.0), ubuf.opacity.x);
}
