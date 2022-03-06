#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 4) vec3 pos;

out vec4 color;


float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

//#define NR_TEXTURES 1

struct Atlas {    
    sampler2D color;
};  

uniform Atlas atlas;


void main()
{
    vec3 normal = normalize(normal_in);

//    color = vec4(vec3(1), 1.0);
//    color = vec4(pos, 1.0);
    color = texture(atlas.color, textureCoordinates);
//    color = vec4(textureCoordinates, 0, 1);
}