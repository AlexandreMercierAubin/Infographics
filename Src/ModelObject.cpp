#pragma once
#include "ModelObject.h"

void ModelObject::Create(GLuint &Program)
{
	program = Program;

	//default settings
	color = glm::vec4(1, 1, 1, 1);
	position = glm::vec3(0.0f,0.0f,0.0f);
	scale = glm::vec3(0.0028f, 0.0028f, 0.0028f);
	ambientIntensity = 0.9f;
	diffuseIntensity = 0.9f;
	model = Model("Resources/megalodon/megalodon.FBX");
}

void ModelObject::Draw(glm::mat4 &perspective, glm::mat4 &view)
{

	glUseProgram(program);
	GLuint MatView = glGetUniformLocation(program, "matView");
	glUniformMatrix4fv(MatView, 1, GL_FALSE, &view[0][0]);
	GLuint MatPerspective = glGetUniformLocation(program, "matPerspective");
	glUniformMatrix4fv(MatPerspective, 1, GL_FALSE, &perspective[0][0]);

	AbstractObject::MatRotation(program, glm::vec3(0,0,0));//roll*pitch*yaw matrix with angles x,y,z
	AbstractObject::MatTranslation(program,position);
	AbstractObject::MatScale(program, scale);

	AbstractObject::uniformColor(program, color);
	AbstractObject::uniformLight(program, glm::vec3(1.0, 1.0, 1.0), glm::vec3(0,0,1), ambientIntensity, diffuseIntensity);


	model.Draw(program);
}
