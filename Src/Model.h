#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;
// GL Includes
#include <GL\glew.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
//#include "..\Dependencies\soil\src\SOIL.h"
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <SDL2\SDL.h>
#include "SDL2\SDL_image.h"
#include "Mesh.h"



class Model
{
public:
	/*  Functions   */
	// Default Constructor
	
	Model();
	// Constructor, expects a filepath to a 3D Model.
	Model(GLchar* path, int colorType);

	// Draws the Model, and thus all its Meshes
	void Draw(GLuint shader);

private:
	/*  Model Data  */
	
	vector<Mesh> Meshes;
	string directory;
	vector<Mesh::Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

										/*  Functions   */
										// Loads a Model with supported ASSIMP extensions from file and stores the resulting Meshes in the Meshes vector.
	void loadModel(string path, int colorType);

	// Processes a node in a recursive fashion. Processes each individual Mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene, int colorType);

	Mesh processMesh(aiMesh* Mesh, const aiScene* scene, int colorType);

	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, int colorType);

	GLint textureFromFile(const char* path, string directory, int colorType);
};

