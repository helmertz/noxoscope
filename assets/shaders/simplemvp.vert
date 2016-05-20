#version 330

layout (location = 0) in vec4 position;
layout (location = 2) in vec2 texCoordIn;

out vec2 texCoord;
out vec3 vsPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main()
{
	mat4 modelViewMatrix = viewMatrix * modelMatrix;
	mat4 modelViewProjectionMatrix = projMatrix * modelViewMatrix;
	vsPosition = (viewMatrix * modelMatrix * position).xyz;
	texCoord = texCoordIn;
	gl_Position = projMatrix * viewMatrix * modelMatrix * position;
}
