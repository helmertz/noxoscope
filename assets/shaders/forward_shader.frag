#version 330

in vec3 vsPosition;
in vec3 viewSpaceNormal;
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D textureDiffuse;
uniform sampler2D textureSpecular;
uniform vec4 colorDiffuse;
uniform bool hasDiffuseTexture;
uniform bool hasSpecularTexture;
uniform float specular;

void main()
{
	vec3 vsNormal = normalize(viewSpaceNormal);
	vec4 matDiffuse = colorDiffuse;

	vec3 matSpecular = vec3(specular);
	if (matDiffuse.a < 0.15) {
		discard;
	}
	vec3 diffuse = matDiffuse.xyz;
	if (hasDiffuseTexture) {
		vec4 diffuseTexCol = texture(textureDiffuse, texCoord);
		if (diffuseTexCol.a < 0.6) {
			discard;
		}
		diffuse *= diffuseTexCol.xyz;
		diffuse *= diffuseTexCol.a;
	}

	if (hasSpecularTexture) {
		vec4 specTexCol = texture(textureSpecular, texCoord);
		matSpecular = specTexCol.xyz;
	}

	vec3 lightPos = vec3(0,0,0);
	vec3 vsViewDir = normalize(-vsPosition);
	vec3 vsLightDir = normalize(lightPos - vsPosition);

	float diffuseFactor = max(dot(vsLightDir,vsNormal), 0.0);

	vec3 h = normalize(vsLightDir + vsViewDir);
	float reflFactor = max(dot(h, vsNormal), 0.0);
	float shininess = 50;
	float specFactor = pow(reflFactor, shininess);
	vec3 color =
		0.60 * diffuse +
		0.80 * diffuse * diffuseFactor +
		0.45 * specFactor * matSpecular
		;

	fragColor = vec4(color, 1.0);
}
