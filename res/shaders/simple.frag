#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;

in layout(location = 8) vec4 lightPos[3];


in vec3 FragPos;
//in vec3 viewPos;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 objectColor = vec3(1);
vec3 currentLightPos = vec3(0, 0, 0);
vec3 result = vec3(0);

float ambientStrenght = 0.1;
vec3 ambientColor = vec3(1, 1, 1);
vec3 ambient = ambientStrenght * ambientColor;

vec3 diffuseColor = vec3(1, 1, 1);
vec3 diffuse = vec3(0);

//float specularStrength = 0.5;
vec3 specular = vec3(0);

float constant = 1.0;
float linear = 0.09;
float quadratic = 0.032;

void main()
{
    // color =  normalize(vec4(1.0, 1.0, -1.0, 1.0)) * vec4(0.5 * normal_in + 0.5, 1.0);
    color =  vec4(0.5 * normal_in + 0.5, 1.0);
    color =  vec4(1.0,0.0,0.0, 1.0);

    vec3 normal = normalize(normal_in);

    for	(int i = 0; i < 2; i++) {

        currentLightPos = lightPos[i].xyz;
        vec3 lightDir = normalize(currentLightPos - FragPos); 

        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);  

        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        //specular = specularStrength * spec * lightColor; 

        //float distance = length(lightDir - FragPos);
        //float attenuation = constant / (1 + linear * distance + quadratic * (distance * distance));   




        //ambient  *= attenuation; 
        //diffuse  *= attenuation;



        //result += (ambient + diffuse + specular);

    }
    
    // result *= objectColor;

    color = vec4(normalize(normal_in), 1.0);
}