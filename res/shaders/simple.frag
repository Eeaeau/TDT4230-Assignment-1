#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 4) vec3 fragPos;

#define NR_POINT_LIGHTS 3

struct PointLight {    
    vec3 position;

    vec3 lightColor;

    float constant;
    float linear;
    float quadratic;
};  

uniform vec3 viewPos;
uniform vec3 ballPos;
uniform vec3 lightTest;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform mat3 normalMatrix;

out vec4 color;


float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 CalcReject(vec3 from, vec3 onto) {
    return from - onto*dot(from, onto)/dot(onto, onto);
}

vec3 currentLightPos = vec3(0, 0, 0);
vec3 result = vec3(0);

float ambientStrenght = 0.0;
vec3 ambientColor = vec3(1, 1, 1);

vec3 diffuseColor = vec3(1);
vec3 emissionColor = vec3(0.0);

float specularStrength = 2;

vec3 lightColor = vec3(4);

//float constant = 0.1;
//float linear = 0.009;
float quadratic = 0.0032; // for some reason uniform did not work for this one


float ballBaseRadius = 1.6;
float ballSoftRadius = ballBaseRadius*1.5;

vec3 CalcPointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir) {
    
        vec3 lightVec = pointLight.position - fragPos; 
        vec3 lightDir = normalize(lightVec); 

        // diffuse shading
        float diff = max(dot(normal, lightDir), 0.0);

        // specular shading
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

        // attuantion
        float lightDist = distance(lightDir, fragPos);
        float attenuation =  1.0/ (pointLight.constant + pointLight.linear * lightDist + quadratic * (lightDist * lightDist));   

        // shadow 

        vec3 ballVec = ballPos - fragPos;
//        vec3 ballDir = normalize(ballVec);
        
        float shadeFactor = 1.0;
        float shadeSoftFactor = 0.0;

        if (length(ballVec)<length(lightVec) && dot(lightVec, ballVec) >= 0) { //branching not optimal for performance
            vec3 reject = CalcReject(ballVec, lightVec);
//            vec3 rejectDir = normalize(reject);
            
            shadeSoftFactor = max(length(reject)-ballSoftRadius, 0);
            shadeSoftFactor = min(shadeSoftFactor, 1);
            
            shadeFactor = max(length(reject)-ballBaseRadius, 0);
            shadeFactor = min(shadeFactor, 1);

            float mixFactor = abs(length(reject)-ballSoftRadius);
            mixFactor = 1/(1+ 0.08*mixFactor*mixFactor);

            shadeFactor *= mix(shadeFactor, shadeSoftFactor, mixFactor);

        }

        
        vec3 diffuse = pointLight.lightColor  * diff * diffuseColor * shadeFactor;
        vec3 specular = pointLight.lightColor * spec * specularStrength * shadeFactor; 

        return (diffuse + specular)*attenuation;

}



void main()
{
    vec3 normal = normalize(normal_in);

    vec3 viewDir = normalize(viewPos - fragPos);

    for	(int i = 0; i < NR_POINT_LIGHTS; i++) {

        result += CalcPointLight(pointLights[i], normal, fragPos, viewDir);

    }

    vec3 ambient = ambientStrenght * ambientColor;

    result += ambient;
    result += emissionColor; 
    result += dither(textureCoordinates);

    color = vec4(result, 1.0);
}