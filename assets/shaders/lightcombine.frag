 #version 330

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gSpecular;
uniform sampler2D gDiffuse;
uniform sampler2D postSSAO;
uniform sampler2D lastFrame;
uniform sampler2D ssrTexture;
uniform sampler2D lightTex;

uniform float screenWidth;
uniform float screenHeight;

uniform float near;
uniform float far;

uniform bool showDebugBar;
uniform bool ssao;
uniform bool ssr;

void main()
{
	float ambientFactor = 0.60;

	vec2 texCoordScaled = texCoord;
	float miniBoxSize = 0;
	float sampleScale = 1.0;

	if (showDebugBar) {
		float numMini = 10.0;
		miniBoxSize = 1.0 / numMini;
		if (texCoord.y < miniBoxSize) {
			texCoordScaled *= numMini;
		}
	}

	// Retrieve data from gbuffer
	vec3 vsPosition = texture(gPosition, texCoordScaled).rgb;
	vec3 vsNormal = texture(gNormal, texCoordScaled).rgb;
	vec3 diffuse = texture(gDiffuse, texCoordScaled).rgb;
	vec3 specular = texture(gSpecular, texCoordScaled).rgb;
	float reflectiveness = texture(gSpecular, texCoordScaled).a;
	float origPosition = texture(gNormal, texCoordScaled).a;

	vec4 ssrColor = ssr ? texture(ssrTexture, texCoordScaled) : vec4(0.0,0.0,0.0,0.0);
	float ssaoFactor = ssao ? texture(postSSAO, texCoordScaled).r : 1.0;
	vec3 lastFrameColor = texture(lastFrame, texCoordScaled).rgb;
	vec3 lightSourceContrib = texture(lightTex, texCoordScaled).rgb;

	vec3 ssrStrength = ssrColor.a * specular;
	vec3 reflectionColor = ssrColor.xyz * ssrStrength + vec3(diffuse) * (1-ssrStrength);

	vec3 ambColor = vec3(1.0,0.96,0.92);
	vec3 color =
		ssaoFactor * (
			0.15 * diffuse * ambColor +
			lightSourceContrib +
			0.3 * reflectionColor
		);

	if (showDebugBar && texCoord.y < miniBoxSize) {
		if (texCoord.x < miniBoxSize) {
			color = vsNormal;
		} else if (texCoord.x < 2 * miniBoxSize) {
			color = diffuse;
		} else if (texCoord.x < 3 * miniBoxSize) {
			color = specular;
		} else if (texCoord.x < 4 * miniBoxSize) {
			color = vec3(origPosition);
		} else if (texCoord.x < 5 * miniBoxSize) {
			color = vec3(lightSourceContrib);
		} else if (texCoord.x < 6 * miniBoxSize) {
			color = vec3(reflectiveness);
		} else if (texCoord.x < 7 * miniBoxSize) {
			color = vec3(ssrColor.xyz);
		} else if (texCoord.x < 8 * miniBoxSize) {
			color = vec3(ssrColor.a);
		} else if (texCoord.x < 9 * miniBoxSize) {
			color = vec3(ssaoFactor);
		} else if (texCoord.x < 10 * miniBoxSize) {
			color = vec3(lastFrameColor);
		} else {
			color = vec3(0);
		}
	}
	fragColor = vec4(color, 1.0);
}
