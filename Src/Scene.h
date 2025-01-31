#pragma once
#include <glm\glm.hpp>
#include "Model.h"
#include <iostream>
#include <array>
#include <stdio.h>
#include <vector>
#include "QuadObject.h"
#include "AbstractObject.h"
#include "SkyboxObject.h"
#include "CubeObject.h"
#include "ShaderLoader.h"
#include "SkyboxShader.h"
#include "TexShader.h"
#include "PrimitiveShader.h"
#include "SimpleGPShader.h"
#include "ModelObject.h"
#include "GroupObject.h"
#include "LightObject.h"
#include "MirrorObject.h"
#include "ModelShader.h"
#include "ModelShaderLambert.h"
#include "ModelShaderBlinnPhong.h"
#include "ParametricCurveObject.h"



#include <random>

class Scene
{

public:

	enum PROJECTIONTYPE
	{
		Perspective,
		InversePerspective,
		Orthographic
	};

	Scene(void);
	~Scene(void);

	void setupScene();
	void setupLight();
	void setupMirrors();
	void setProjection(PROJECTIONTYPE type, const float & angleOfView = 0.5236f, const float & aspect = 1.5f, const float & near = 0.05f, const float &far = 100.0f);
	void addYaw(GLfloat y);
	void addPitch(GLfloat z);
	void setPosition(glm::vec3 pos);

	void drawSkybox();
	void drawScene();
	void drawMirrors(int w,int h,bool drawSkybox);

	void mouseMotion(const unsigned int & timestamp, const unsigned int & windowID, const unsigned int & state, const int & x, const int & y, const int & xRel, const int & yRel);

	void addObject(shared_ptr<AbstractObject> object);

	void dollyZoom(float dolly, float zoom);

	void renderRaycast(const int &width, const int &height, const int &rayPerPixel, const int &depth, const string &fileName);

	void setFog(bool value);
	bool getFog();

	struct KeyFlags
	{
		bool flagUp, flagDown, flagLeft, flagRight, flagLeftMouse, flagRightMouse;
	};
	void refreshScene(KeyFlags flags);

	std::shared_ptr<GroupObject> getObjects();

private:

	PROJECTIONTYPE projectionType;

	struct objetMonde;
	const float sensitivity = 0.2f;
	const float mouvementSpeed = 0.03f;
	float viewAngle = 30.0f;

	void MatPerspective(glm::mat4 &proj, const float &angleOfView, const float &aspect, const float &near, const float &far);
	void MatInversePerspective(glm::mat4 &proj, const float &angleOfView, const float &aspect, const float &near, const float &far);
	void MatOrthographic(glm::mat4 &proj, const float &angleOfView, const float &aspect, const float &near, const float &far);
	void MatView(glm::mat4 &matView, bool staticPos);
	glm::mat4 projection;
	glm::mat4 view;

	glm::vec3 direction;
	glm::vec4 uniformCouleur;

	glm::vec3 position;
	glm::vec3 orientation;
	GLfloat yaw;
	GLfloat pitch;

	//temp test change for an array for scene graph

	CubeObject cube;
	Model g_requinModel;
	GLuint shaderID;
	GLuint skyboxID;
	GLuint bufferID;
	GLuint bufferColorID;
	GLuint tessellationShaderID;
	GLuint texShaderID;
	//end of test
	std::shared_ptr<GroupObject> objects;
	std::vector<AbstractObject::Light*> lights;
	std::vector<std::shared_ptr<MirrorObject>> mirrors;
	SkyboxObject skybox;

	// Raycasting
	std::random_device rd;
	std::mt19937 rng{ rd() };
	std::uniform_real_distribution<double> random01{ 0.0, 1.0 };
	const double gammaCorrection = 1 / 2.2;

	bool raycast(const Ray &ray, double &distance, glm::vec3 &normal, std::shared_ptr<AbstractObject> &object);
	glm::vec4 computeRadiance(const Ray &ray, const int &depth, const int &maxDepth);
	glm::vec4 makeLightPoint(const glm::vec4 &color, const float &shininess, const glm::vec3 &normal, glm::vec3 &vAmbient,
		const glm::vec3 &surfaceToLight, const glm::vec3 &surfaceToCamera, const float &attenuation, AbstractObject::Light* light);
	void saveImageFile(const int &width, const int &height, glm::vec4* pixels, const string &fileName);
	int formatColorComponent(double value);
};
