#version 330

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gDiffuse;
layout (location = 3) out vec4 gSpecular;
layout (location = 4) out vec4 gNormalMappedNormal;

in vec3 vsPosition;
in vec3 viewSpaceNormal;
in vec3 viewSpaceTangent;
in vec3 viewSpaceBitangent;
in vec2 texCoord;

uniform sampler2D textureDiffuse;
uniform sampler2D textureSpecular;
uniform sampler2D textureNormal;
uniform vec4 colorDiffuse;
uniform bool hasDiffuseTexture;
uniform bool hasSpecularTexture;
uniform bool hasNormalTexture;
uniform float specular;
uniform float reflectiveness;
uniform float texRepeatFactor;
uniform float near;
uniform float far;

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	vec2 texCoordS = texRepeatFactor * texCoord;
	vec3 vsNormal = normalize(viewSpaceNormal);
	vec4 matDiffuse = colorDiffuse;

	vec3 matSpecular = vec3(specular);
	if (matDiffuse.a < 0.15) {
		discard;
	}
	vec3 diffuse = matDiffuse.xyz;
	if (hasDiffuseTexture) {
		vec4 diffuseTexCol = texture(textureDiffuse, texCoordS);
		if (diffuseTexCol.a < 0.6) {
			discard;
		}
		diffuse *= diffuseTexCol.xyz;
		diffuse *= diffuseTexCol.a;
	}

	if (hasSpecularTexture) {
		vec4 specTexCol = texture(textureSpecular, texCoordS);
		matSpecular = specTexCol.xyz;
	}

	gPosition = vec4(vsPosition, linearizeDepth(gl_FragCoord.z));
	gNormal = vec4(vsNormal, gl_FragCoord.z);
	gDiffuse.rgb = diffuse;

	if (hasNormalTexture) {
		vec3 texTSNormal = texture(textureNormal, texCoordS).rgb;
		texTSNormal = normalize(texTSNormal * 2.0 - 1.0);

		gNormalMappedNormal.xyz = normalize(
			texTSNormal.x * normalize(viewSpaceTangent) +
			texTSNormal.y * normalize(viewSpaceBitangent) +
			texTSNormal.z * normalize(viewSpaceNormal)
		);
	} else {
		gNormalMappedNormal.xyz = gNormal.xyz;
	}

	gSpecular.rgb = matSpecular;
	gSpecular.a = reflectiveness;
}
