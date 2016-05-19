#version 330

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D tex;

void main()
{
	vec4 sample = texture(tex, texCoord);
	fragColor = sample;
}
