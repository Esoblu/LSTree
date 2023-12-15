#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 nor;
    vec2 TexCoord;
    vec4 FragPosLightSpace;
} fs_in;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDir;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D shadowMap;
uniform bool shadows;
uniform float mix_rate;
uniform float lightStrength;

// vec3 lightDir = normalize(lightPos - fs_in.FragPos);
vec3 normal = normalize(fs_in.nor);
float ShadowCalculation(vec4 FragPosLightSpace){
	// perform perspective divide
	vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
	// transform to [0,1] range
	projCoords = projCoords * 0.5 + 0.5;
	// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r; 
	// get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	// check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(fs_in.nor, lightDir)), 0.005);
    float shadow = 0.0f;

	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
	if(projCoords.z > 1.0f || projCoords.z < 0.0f)
		shadow = 0.0f;
    return shadow;

}

void main(){
	vec4 color = mix(texture(texture1, fs_in.TexCoord), texture(texture2, fs_in.TexCoord), mix_rate);
	if(color.a<0.01f){
		discard;
	}
	float ambientStrength = 0.5f;
	float specularStrength = 0.5f;
	float attenuation_x = 1.0f;
	float attenuation_y = 0.09f;
	float attenuation_z = 0.032f;
	vec3 ambient = ambientStrength * lightColor;

	float diff = max(dot(normal, lightDir), 0.0f);
	vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	vec3 halfwayDir = normalize(lightDir + viewDir); 
	float spec = pow(max(dot(normal, halfwayDir), 0.0f), 32);
	vec3 specular = specularStrength * spec * lightColor;

	// float d = distance(lightPos, fs_in.FragPos);

	float attenuation = 1.0f; // (attenuation_x + attenuation_y * d + attenuation_z * pow(d, 2));

	float shadow = shadows? ShadowCalculation(fs_in.FragPosLightSpace): 0.0f;
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(color) * attenuation * lightStrength;
	
	FragColor = vec4(lighting, 1.0);
}