#version 430

in vec2 uv;
out vec4 color;

uniform sampler2D kuwabara;

void main()
{
	//color = vec4(1.0f, uv, 1.0f);
	color = texture2D(kuwabara, uv);
}