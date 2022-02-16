#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 4) vec3 fragPos;

#define NR_POINT_LIGHTS 3  
//layout(location = 6) in vec4 lightPos[NR_POINT_LIGHTS];


struct PointLight {    
    vec3 position;
//    float constant;
//    float linear;
//    float quadratic;  
//
//    vec3 ambient;
//    vec3 diffuse;
//    vec3 specular;
};  

uniform vec3 viewPos;
uniform vec3 lightTest;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform mat3 normalMatrix;


//in vec3 fragPos;
in vec3 aTestVar;

out vec4 color;


float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 objectColor = vec3(1);
vec3 currentLightPos = vec3(0, 0, 0);
vec3 result = vec3(0);

float ambientStrenght = 0.1;
vec3 ambientColor = vec3(1, 1, 1);


vec3 diffuseColor = vec3(1, 1, 1);

float specularStrength = 0.5;

vec3 lightColor = vec3(1, 0, 0);

float constant = 1.0;
float linear = 0.0009;
float quadratic = 0.00032;


vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir) {
    
        vec3 lightDir = normalize(pointLight.position - fragPos); 

        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);

        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        // attuantion
        float distance = length(lightDir - fragPos);
        float attenuation = constant / (1 + linear * distance + quadratic * (distance * distance));   

        // combine shaders in result 

        vec3 ambient = ambientStrenght * ambientColor;
        vec3 diffuse = lightColor  * diff * diffuseColor;
        vec3 specular = lightColor * spec * specularStrength; 

        ambient  *= attenuation;
        diffuse  *= attenuation;
        specular *= attenuation;

        return (ambient + diffuse + specular);

}

void main()
{
//    color =  normalize(vec4(1.0, 1.0, -1.0, 1.0)) * vec4(0.5 * normal_in + 0.5, 1.0);
//    color =  vec4(0.5 * normal_in + 0.5, 1.0);
//    color =  vec4(1.0,0.0,0.0, 1.0);

    vec3 normal = normalize(normal_in);
//    vec3 normal = normalize(normalMatrix * normal_in);

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0.2);

    for	(int i = 0; i < NR_POINT_LIGHTS; i++) {

        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir);

    }

    color = vec4(result, 1.0);
}