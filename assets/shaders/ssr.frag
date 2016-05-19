#version 330

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D gPosition;
uniform sampler2D gSpecular;
uniform sampler2D gNormal;
uniform sampler2D lastFrame;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

const float reflectionEdgeSmoothing = 3;

void main()
{
	// Retrieve data from gbuffer
	vec3 vsPosition = texture(gPosition, texCoord).rgb;
	vec3 vsNormal = texture(gNormal, texCoord).rgb;
	float reflectiveness = texture(gSpecular, texCoord).a;

	vec3 vsViewDir = normalize(-vsPosition);

	vec3 reflectDir = reflect(-vsViewDir, vsNormal);

	float reflDist = 20;
	float deltaL1 = 0.15;
	float deltaL2 = 0.001;
	float bias = 0.08;

	int numMarchesL1 = int(reflDist / deltaL1) ;
	vec3 stepL1 = deltaL1 * reflectDir;

	vec3 hitCoord = vsPosition;
	vec4 reflectionColor = vec4(0);

	if (reflectiveness < 0.01) {
		fragColor = reflectionColor;
		return;
	}
	// March along reflection until geometry is hit
	for (int i = 0; i < numMarchesL1; i++) {
		vec3 hitPreUpdate = hitCoord;
		hitCoord += stepL1;

		vec4 hitCoordTex = projMatrix * vec4(hitCoord, 1.0);
		hitCoordTex.xy /= hitCoordTex.w;
		hitCoordTex.xy = hitCoordTex.xy * 0.5 + 0.5;
		if (hitCoordTex.x < 0 || hitCoordTex.x > 1 || hitCoordTex.y < 0 || hitCoordTex.y > 1) {
			break;
		}

		float reflDepth = texture(gPosition, hitCoordTex.xy).z;
		float depthDist = reflDepth - hitCoord.z;

		if (depthDist > 0) {
			// Do binary search in step 2
			vec3 low = hitPreUpdate;
			vec3 high = hitCoord;

			while (length(high - low) > deltaL2) {
				hitCoord = low + (high - low) / 2;
				vec4 hitCoordProj = projMatrix * vec4(hitCoord, 1.0);
				hitCoordTex = hitCoordProj;
				hitCoordTex.xy /= hitCoordProj.w;
				hitCoordTex.xy = hitCoordTex.xy * 0.5 + 0.5;
				reflDepth = texture(gPosition, hitCoordTex.xy).z;
				depthDist = reflDepth - hitCoord.z;
				if (depthDist > 0.0) {
					high = hitCoord;
				} else {
					low = hitCoord;
				}
			}
			if (depthDist > 2 * deltaL2) {
				break;
			}
			hitCoord = high;

			// Smooth transition around edges of sampling texture
			float edgescale = 1;
			edgescale *= clamp(reflectionEdgeSmoothing * hitCoordTex.y,0,1);
			edgescale *= clamp(reflectionEdgeSmoothing * (1 - hitCoordTex.y),0,1);
			edgescale *= clamp(reflectionEdgeSmoothing * hitCoordTex.x,0,1);
			edgescale *= clamp(reflectionEdgeSmoothing * (1 - hitCoordTex.x),0,1);

			reflectionColor.xyz = texture(lastFrame, hitCoordTex.xy).rgb;
			reflectionColor.a = reflectiveness * edgescale;
			break;
		}
	}

	fragColor = reflectionColor;
}