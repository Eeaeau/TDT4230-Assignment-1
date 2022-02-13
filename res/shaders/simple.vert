#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform layout(location = 3) mat4 MVP;
uniform layout(location = 4) mat4 MV;
// uniform layout(location = 7) vec4 lightPos;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 v_out;
// out layout(location = 7) vec3 lightPos_out;


void main()
{
    normal_out = normal_in;
    textureCoordinates_out = textureCoordinates_in;
    gl_Position = MVP * vec4(position, 1.0f);
    v_out = vec3(MV * vec4(position, 1.0f));
}