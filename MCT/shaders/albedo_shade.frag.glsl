#version 430

in vec3 vPos;
in vec2 vTex;
in vec3 vNormal;

out vec4 color;

layout(binding = 0) uniform sampler2D texture_diffuse1;
layout(binding = 1) uniform sampler2D mesh_color;

void main()
{
	//vec3 normal = normalize(vNormal);
	//vec3 position = vPos;
	//
	//vec3 light_dir = normalize(vec3(0.0f, 1.0f, 1.0f) - position);
	//
	//vec3 albedo = texture2D(texture_diffuse1, vTex).rgb;
	//
	//vec3 ambient = albedo * 0.3f;
	//vec3 diffuse = albedo * max(0.0f, dot(normal, light_dir));
	//
	//
	//color = vec4(ambient + diffuse, 1.0f);

	//color = texture2D(texture_diffuse1, vTex);
	color = texture2D(mesh_color, vTex);
	

}

