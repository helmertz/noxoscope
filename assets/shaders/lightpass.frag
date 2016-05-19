#version 330

in vec2 texCoord;

out vec4 fragColor;

uniform bool stencilDebugRender;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gSpecular;
uniform sampler2D gDiffuse;

uniform float near;
uniform float far;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightStrength;

void main()
{
	vec3 vsPosition = texture(gPosition, texCoord).rgb;
	vec3 vsNormal = texture(gNormal, texCoord).rgb;
	vec3 diffuse = texture(gDiffuse, texCoord).rgb;
	vec3 specular = texture(gSpecular, texCoord).rgb;
	float reflectiveness = texture(gSpecular, texCoord).a;

	vec3 vsViewDir = normalize(-vsPosition);
	vec3 lightDiff = lightPos - vsPosition;
	float lightDist = length(lightDiff);
	vec3 vsLightDir = normalize(lightDiff);

	float diffuseFactor = max(dot(vsLightDir, vsNormal), 0.0);

	vec3 h = normalize(vsLightDir + vsViewDir);
	float reflFactor = max(dot(h, vsNormal), 0.0);
	float shininess = 110;
	float specFactor = pow(reflFactor, shininess);

	float lightIntensity = lightStrength;
	float lightDistFactor = lightIntensity * lightDist + 1;

	vec3 color = lightDistFactor * (
		diffuse * lightColor * diffuseFactor +
		specFactor * specular * lightColor
	);

	if (stencilDebugRender) {
		color = 0.2 * lightColor;
	}
	fragColor = vec4(color, 1.0);
}
