#pragma once
#include "AbstractShader.h"


class SkyboxShader : public AbstractShader
{
	std::string fshader =
R"(#version 430 core
in vec3 TexCoords;
out vec4 color;
uniform samplerCube skybox;
uniform vec4 Color;

void main()
{

	color = texture(skybox, TexCoords)*Color;

}

)";




	std::string vshader =
R"(#version 430 core 
layout(location = 0) in vec3 position;
uniform mat4 matPerspective;
uniform mat4 matView;
out vec3 TexCoords;

void main() 
{
	
	vec4 pos = matPerspective * matView * vec4(position, 1.0);
	gl_Position = pos.xyww;
	TexCoords = normalize(position);
}
)";

public:
	virtual std::string getFragmentShader()
	{
		return fshader;
	}
	
	virtual std::string getVertexShader()
	{
		return vshader;
	}

};

