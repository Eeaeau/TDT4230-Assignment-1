#version 450 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;

//uniform layout(location = 3) mat4 MVP;
//uniform layout(location = 5) vec3 normalMatrix_in;

// uniform layout(location = 7) vec4 lightPos;

uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelMatrix;
uniform vec3 viewPos;
//uniform vec3 tangent_in;


out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 4) vec3 fragPos;
out layout(location = 5) mat3 TBN;
out layout(location = 9) vec3 tangent_out;

// out layout(location = 7) vec3 lightPos_out;



void main()
{
//    vec3 bitangent = normalize(normalMatrix * cross(normal_in, tangent_in));
    vec3 bitangent = normalize(normalMatrix * bitangent_in);

   tangent_out = normalize( normalMatrix * tangent_in);
//    tangent_out = normalize( vec3(modelMatrix * vec4(tangent_in, 1.0f)));

	normal_out = normalize(normalMatrix * normal_in);
    
    TBN = mat3(
		 normalize(tangent_in),
		 normalize(bitangent),
		 normalize(normal_out) 
	);

//   vec3 T = normalize(vec3(modelMatrix * vec4(tangent_in,   0.0)));
//   tangent_out = T;
//   vec3 B = normalize(vec3(modelMatrix * vec4(bitangent_in, 0.0)));
//   vec3 N = normalize(vec3(modelMatrix * vec4(normal_in,    0.0)));
//   mat3 TBN = mat3(T, B, N);

//	normal_out = normal_in;

    textureCoordinates_out = textureCoordinates_in;

    gl_Position = MVP * vec4(position, 1.0f);
    
    fragPos = vec3(modelMatrix * vec4(position, 1.0f));
}