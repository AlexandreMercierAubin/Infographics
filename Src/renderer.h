#pragma once
#include "SDL2/SDL.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<vector>
#include "shader_loader.h"

#include "kochFragmentShader.h"
#include "kochVertexShader.h"

class Renderer
{
public:
	void setupRenderer(SDL_Window * window, SDL_GLContext *context);
	void drawRenderer();
	void deleteRenderer();
private:
	SDL_Window * window;
	SDL_GLContext gl;
	SDL_Renderer *sdlRenderer;
	GLuint shaderID;
	GLuint bufferID;
	GLuint bufferColorID;

	std::vector<glm::vec3> BackgroundColor;
	std::vector<glm::vec3> Lines; 
	std::vector<glm::vec3> Colors;
	
	GLuint matRotation;
	GLuint matScale;
	GLuint matTranslation;

	float testScale;

	void initShaders();
	void MatScale();
	void MatRotation();
	void MatTranslation(int index);

	//testfunc
	void courbeKoch(glm::vec3 pointDebut, glm::vec3 pointFin, int nbIterations);
};
