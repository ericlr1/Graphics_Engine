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
	float spec = pow(max(dot(aViewDir, reflectDir), 0.0), 2.0);

	float distance = length(aLight.position - aPosition);

	float constant = 1.0;
	float linear = 0.09;
	float quadratic = 0.032;
	float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

	vec3 ambient = aLight.color * 0.2;
	vec3 diffuse = aLight.color * diff;
	vec3 specular = 0.1 * spec * aLight.color;
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 CalcDirLight(Light aLight, vec3 aNormal, vec3 aViewDir)
{
    vec3 lightDir = normalize(-aLight.direction);
    float diff = max(dot(aNormal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, aNormal);
    float spec = pow(max(dot(aViewDir, reflectDir), 0.0), 2.0);
    
    vec3 ambient = aLight.color * 0.2;
    vec3 diffuse = aLight.color * diff;
    vec3 specular = 0.1 * spec * aLight.color;

    return (ambient + diffuse + specular);
}

void main()
{
	vec3 Albedo = vec3(texture(uAlbedo, vTexCoord));
	vec3 Normal = texture(uNormals, vTexCoord).xyz;
	vec3 ViewDir = texture(uViewDir, vTexCoord).xyz;
	vec3 Position = texture(uPosition, vTexCoord).xyz;


	vec3 returnColor = vec3(0.0);

	for(int i = 0; i < uLightCount; ++i)
	{
		vec3 lightResult = vec3(0.0);
		if(uLight[i].type == 0)
		{
			lightResult = CalcDirLight(uLight[i], Normal, ViewDir);
		}
		else if(uLight[i].type == 1)
		{
			lightResult = CalcPointLight(uLight[i], Normal, Position, ViewDir);
		}
		returnColor.rgb += lightResult * Albedo.rgb;
		
	}

	oColor = vec4(returnColor, 1.0);
}

#endif
#endif