#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

//uniform mat4 MV;
//uniform mat4 P;

uniform mat4 MVP;

out vec3 vPos;
out vec2 vTex;
out vec3 vNormal;

void main()
{
	//vec3 viewPos = vec3(MV * (position,1.0f));
	//gl_Position = P * vec4(viewPos, 1.0f);

	gl_Position = MVP * vec4(position, 1.0f);

	vPos = position;
	vTex = uv;
	vNormal = normal;
}



