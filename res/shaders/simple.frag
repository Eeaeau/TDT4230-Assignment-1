#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 v;

in layout(location = 8) vec4 lightPos[3];

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }


void main()
{
    // color =  normalize(vec4(1.0, 1.0, -1.0, 1.0)) * vec4(0.5 * normal_in + 0.5, 1.0);
    color =  vec4(0.5 * normal_in + 0.5, 1.0);
    color =  vec4(0.5 * normal_in + 0.5, 1.0);
}