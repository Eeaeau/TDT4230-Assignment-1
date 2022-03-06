#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
//in layout(location = 4) vec3 pos;

out vec4 color;

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