#version 450 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

//uniform layout(location = 3) mat4 MVP;
//uniform layout(location = 5) vec3 normalMatrix_in;

// uniform layout(location = 7) vec4 lightPos;

uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
uniform vec3 viewPos;


out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 4) vec3 fragPos;
// out layout(location = 7) vec3 lightPos_out;
//out vec3 fragPos;
//out vec3 aTestVar;


void main()
{

	normal_out = normalize(normalMatrix * normal_in);
//	normal_out = normal_in;

    textureCoordinates_out = textureCoordinates_in;

    gl_Position = MVP * vec4(position, 1.0f);
    // gl_NormalMatrix = normalMatrix
    
    fragPos = vec3(modelMatrix * vec4(position, 1.0f));
//    fragPos = vec3(0.5);
//    aTestVar = vec3(0.2);
//    fragPos = viewPos;
}