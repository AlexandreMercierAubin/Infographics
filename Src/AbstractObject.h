#pragma once
#include <glm\glm.hpp>
#include "Model.h"
#include <sstream>
#include <string>
#include<cmath>
#include "Ray.h"
#include <memory>

class AbstractObject
{
public:
	//glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 1.0f, 0.0f),glm::vec3(-1.0f, 1.0f, 0.0f), 0.2f, 1.0f
	struct Light
	{
		int lightType = 1;

		glm::vec3 ambientColor=glm::vec3(1.0f, 1.0f, 1.0f);
		float ambientIntensity=0.100f;

		glm::vec3 diffuseColor= glm::vec3(1, 1, 1);
		float diffuseIntensity=1.0f;

		glm::vec3 specularColor = glm::vec3(1, 1, 1);
		float specularIntensity = 0.5f;
		float attenuation = 0.2f;

		glm::vec3 volume = glm::vec3(1, 1, 1);//cubic volume
		bool volumetric = true;

		glm::vec3 direction = glm::vec3(-1.0f, -1.0f, 0.0f);
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
		float coneAngle = 30.0f;
	};


	virtual void Create(GLuint &Program) {}
	virtual void Draw(glm::mat4 &projection, glm::mat4 &view, glm::vec3 &camPos, const vector<Light*> &lights) {}
	virtual ~AbstractObject() {};
	AbstractObject();

	virtual void setPosition(glm::vec3 pos);
	virtual void addPosition(const glm::vec3 &pos);
	virtual glm::vec3 getPosition();

	virtual void setRotationDegree(glm::vec3 rot);
	virtual void addRotationDegree(const glm::vec3 &rot);
	virtual  glm::vec3 getRotationDegree();
	virtual void setRotationQuaternion(glm::quat quat);
	virtual void addRotationQuaternion(const glm::quat &quat);
	virtual  glm::quat getRotationQuaternion();

	virtual void setScale(glm::vec3 scl);
	virtual void addScale(const glm::vec3 &scl);
	virtual  glm::vec3 getScale();

	virtual void setColor(glm::vec4 Color);
	virtual glm::vec4 getColor();

	virtual void setShininess(float shininess);
	virtual void addShininess(const float &shininess);
	virtual float getShininess();

	virtual void setFog(bool value);
	virtual bool getFog();

	virtual bool raycast(const Ray &ray, double &distance, glm::vec3 &normal, std::shared_ptr<AbstractObject> &object) { return false; }

	bool isSelected();
	void setSelected(bool value);
	string getName();

protected:

	string name = "Abstract";

	virtual void uniformColor(GLuint &program, glm::vec4 &uniformColor);
	virtual void uniformCameraPosition(GLuint &program, glm::vec3 &uniformCameraPos);
	virtual void uniformLight(GLuint &program,vector<Light*>lights);
	virtual void uniformShininess(GLuint &program,float shininess);
	virtual void uniformFog(GLuint &program, bool value);

	virtual void MatRotation(const GLuint &program, glm::mat4 &rotat, const glm::vec3 &r);
	virtual void MatRotationDegree(const GLuint &program, glm::mat4 &rotat, const glm::vec3 &r);
	virtual void MatRotationQuaternion(const GLuint &program, glm::mat4 &rotat, const glm::quat &r);
	virtual void MatTranslation(GLuint program, glm::mat4 &trans, const glm::vec3 &position);
	virtual void MatScale(GLuint program, glm::mat4 &scl, const glm::vec3 &scale);

	glm::vec4 color;
	glm::vec3 rotationDegree;
	glm::quat rotationQuat;
	glm::vec3 position;
	glm::vec3 scale;
	
	float shininess;
	bool useFog = false;

	GLuint program;

private:
	bool selected = false;



};
