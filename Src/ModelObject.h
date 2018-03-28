#pragma once
#include "AbstractObject.h"
#include "Model.h"
#include "LightObject.h"

class ModelObject: public AbstractObject
{
public:
	virtual void Create(GLuint &Program);
	virtual void Draw(glm::mat4 &projection, glm::mat4 &view, glm::vec3 &camPos, const vector<Light*>& lights);
	void setModelToCreate(string path);
	//virtual void SetColor(glm::vec4 color);
private:
	string modelPath = "Resources/Megalodon/megalodon.FBX";
	Model model;
};
