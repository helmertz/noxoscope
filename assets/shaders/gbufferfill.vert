#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec3 normalIn;
layout (location = 2) in vec3 tangentIn;
layout (location = 3) in vec3 bitangentIn;
layout (location = 4) in vec2 texCoordIn;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 viewSpaceNormal;
out vec3 viewSpaceTangent;
out vec3 viewSpaceBitangent;
out vec2 texCoord;
out vec3 vsPosition;

void main()
{
	mat4 modelViewMatrix = viewMatrix * modelMatrix;
	mat4 modelViewProjectionMatrix = projMatrix * modelViewMatrix;
	mat4 normalMatrix = inverse(transpose(modelViewMatrix));
	viewSpaceNormal = vec3(normalize((normalMatrix * vec4(normalIn,0.0)).xyz));
	viewSpaceTangent = vec3(normalize((normalMatrix * vec4(tangentIn,0.0)).xyz));
	viewSpaceBitangent = vec3(normalize((normalMatrix * vec4(bitangentIn,0.0)).xyz));
	vsPosition = (viewMatrix * modelMatrix * position).xyz;
    texCoord = texCoordIn;
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
}
