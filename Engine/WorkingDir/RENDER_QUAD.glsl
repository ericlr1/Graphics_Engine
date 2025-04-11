
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#ifdef RENDER_QUAD

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

struct Light
{
	unsigned int type;
	vec3 color;
	vec3 direction;
	vec3 position;
};

layout(binding=0, std140) uniform GlobalParams
{
	vec3 uCameraPosition;
	unsigned int uLightCount;
	Light uLight[16];
};

in vec2 vTexCoord;

uniform sampler2D uAlbedo;
uniform sampler2D uNormals;
uniform sampler2D uPosition;
uniform sampler2D uViewDir;

layout(location=0) out vec4 oColor;

vec3 CalcPointLight(Light aLight, vec3 aNormal, vec3 aPosition, vec3 aViewDir)
{
	vec3 lightDir = normalize(aLight.position - aPosition);
	float diff = max(dot(aNormal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, aNormal);
	float spec = pow(max(dot(normalize(aViewDir), reflectDir), 0.0), 32.0); // Aumentar brillo

	float distance = length(aLight.position - aPosition);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

	vec3 ambient = aLight.color * 0.1;
	vec3 diffuse = aLight.color * diff;
	vec3 specular = 0.5 * spec * aLight.color; // Aumentar especular
	return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcDirLight(Light aLight, vec3 aNormal, vec3 aViewDir)
{
    vec3 lightDir = normalize(-aLight.direction);
    float diff = max(dot(aNormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(normalize(aViewDir), reflectDir), 0.0), 32.0);
    
    vec3 ambient = aLight.color * 0.1;
    vec3 diffuse = aLight.color * diff;
    vec3 specular = 0.5 * spec * aLight.color;
    return (ambient + diffuse + specular);
}

void main()
{
	vec3 Albedo = texture(uAlbedo, vTexCoord).rgb;
	vec3 Normal = normalize(texture(uNormals, vTexCoord).rgb); // Normalizar
	vec3 Position = texture(uPosition, vTexCoord).rgb;
	vec3 ViewDir = normalize(texture(uViewDir, vTexCoord).rgb); // Normalizar

	vec3 result = vec3(0.0);
	for(int i = 0; i < uLightCount; ++i)
	{
		if(uLight[i].type == 0)
			result += CalcDirLight(uLight[i], Normal, ViewDir) * Albedo;
		else if(uLight[i].type == 1)
			result += CalcPointLight(uLight[i], Normal, Position, ViewDir) * Albedo;
	}

	oColor = vec4(result, 1.0);
}

#endif
#endif