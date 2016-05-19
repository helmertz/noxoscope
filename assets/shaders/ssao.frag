#version 330

in vec2 texCoord;

out float outShading;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform mat4 invProj;
uniform int width;
uniform int height;

const int kernelSize = 32;
uniform vec3 samples[kernelSize];

uniform mat4 projMatrix;

uniform float near;
uniform float far;

void main()
{
	vec3 vsPos = texture(gPosition, texCoord).xyz;
	float depth = texture(gPosition, texCoord).a;
	vec3 normal = texture(gNormal, texCoord).rgb;
	vec2 noiseScale = vec2(width, height) / 2;

	vec3 rvec = texture(texNoise, texCoord * noiseScale).xyz * 2.0 - 1.0;
	vec3 tangent = normalize(rvec - normal * dot(rvec, normal) );
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn = mat3(tangent, bitangent, normal);
    float radius = 0.3;

	float occlusion = 0.0;
	for (int i = 0; i < kernelSize; i++) {
		vec3 sample = tbn * samples[i];
		sample = sample * radius + vsPos;

		vec4 offset = vec4(sample, 1.0);
		offset = projMatrix * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		float sampleDepth = texture(gPosition, offset.xy).z;

		float rangeCheck = abs(vsPos.z - sampleDepth) < radius ? 1.0 : 0.0;
		occlusion += (sampleDepth > sample.z ? 1.0 : 0.0) * rangeCheck;
	}
	outShading = 1.0 - (occlusion / kernelSize);
}
