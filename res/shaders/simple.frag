#version 430 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 v;

in layout(location = 7) vec3 lightPos;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }


void main()
{
    int n_lights = 1;
    //color = ambient * normalize(vec4(1.0, 1.0, -1.0, 1.0)) * vec4(0.5 * normal + 0.5, 1.0);


	float shininess = 100.0f;
	float lightReach = 0.003f;

	// vectors
	vec3 eye_direction = normalize(-v);
	vec3 light_direction, reflected_light_dir;
	vec4 diff_comp, spec_comp;
	vec2 light_intensity = vec2(1.1, 0.5); // how much light each light contributes

	// colors
	color = vec4(0, 0, 0, 1);
	vec4 ambient = vec4(0.1);
	vec4 emission = vec4(0); 
	vec4 specular_color = vec4(1); // white
	vec4 light_color = vec4(0);
	vec4 textureColour = vec4(1, 1, 1, 1);

	vec3 normal = normalize(normal_in);

    for (int i = 0; i < n_lights; i++) {

		vec3 surfaceLightVector = lightPos[i] - v;
		float lightDistance = length(surfaceLightVector);
		light_direction = normalize(surfaceLightVector);
		reflected_light_dir = normalize(reflect(-light_direction, normal));

		// diffuse part
		float diffuse_intensity = max(dot(normal, light_direction), 0.0f);
		diff_comp = vec4(textureColour.rgb*diffuse_intensity, 1.0);

		// specular part
		spec_comp = clamp(pow(max(dot(reflected_light_dir, eye_direction), 0.0f), shininess)*specular_color, 0.0f, 0.05f); // clamps down the shinyness

		light_color += ((diff_comp + spec_comp) / (lightReach * lightDistance))*light_intensity[i];

	}
	color +=  ambient + emission + 0.4* light_color;
}