#version 330

in vec2 texCoord;

out float fragColor;

uniform sampler2D tex;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(tex, 0));
	float result = 0.0;
	for (int x = -2; x < 2; ++x)
	{
		for (int y = -2; y < 2; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize;
			result += texture(tex, texCoord + offset).r;
		}
	}
	fragColor = result / (4.0 * 4.0);
}
