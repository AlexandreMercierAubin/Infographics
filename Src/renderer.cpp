#pragma once
#include "Renderer.h"



void Renderer::setupRenderer(SDL_Window * window, SDL_GLContext *context)
{
	this->window = window;

	#ifndef __APPLE__
		glewExperimental = GL_TRUE;
		glewInit();
	#endif

	initShaders();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	scene.setupScene();

	BackgroundColor = glm::vec3(0.0f, 0.0f, 0.0f);
	couleurRemplissage = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	couleurBordure = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	srand(static_cast <unsigned> (time(0)));
	testScale = 0;

	currentTranslation = glm::vec3(0.0f, 0.0f, 0.0f);
	currentRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	currentScale = glm::vec3(1.0f, 1.0f, 1.0f);

	lightAmbientColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightDiffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightSpecularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	lightDirection = glm::vec3(0.0f, -1.0f, 0.0f);

	// Setup ImGUI
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	ImGui_ImplSdlGL3_Init(window);
	ImGui::StyleColorsDark();
}

void Renderer::initShaders()
{
	Core::ShaderLoader loader;
	
	PrimitiveShader primitiveShader;
	primitiveShaderID = loader.CreateProgram(primitiveShader);
	TexShader texShader;
	texShaderID = loader.CreateProgram(texShader);
	ModelShader modelShader;
	modelShaderPhongID = loader.CreateProgram(modelShader);
	ModelShaderLambert modelShaderLambert;
	modelShaderLambertID = loader.CreateProgram(modelShaderLambert);
	ModelShaderBlinnPhong modelShaderBlinnPhong;
	modelShaderBlinnPhongID = loader.CreateProgram(modelShaderBlinnPhong);
	currentModelShaderID = modelShaderPhongID;
	SimpleGPShader GPShader;
	GPShaderID = loader.CreateProgram(GPShader);
	PostProcessShader postProcessShader;
	postProcessShaderID = loader.CreateProgram(postProcessShader);
	TessellationCEShader tessCe;
	TessellationShader tess;
	tessellationShaderID = loader.CreateProgramTess(tess, tessCe);
	PostProcessColorShader ppColorShader;
	postProcessColorShaderID = loader.CreateProgram(ppColorShader);

	curseur.Create(primitiveShaderID);
	curseur.setCouleurRemplissage(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	curseur.setCouleurBordure(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

}



void Renderer::drawRenderer(Scene::KeyFlags &flags)
{
	if (MSAA) 
	{
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
		glDisable(GL_LINE_SMOOTH);
	}

	if (activateWireframe) 
	{
		glEnable(GL_POLYGON_SMOOTH);
	}
	else 
	{
		glDisable(GL_POLYGON_SMOOTH);
	}

	scene.refreshScene(flags);

	if (scene.getFog())
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	else
		glClearColor(BackgroundColor[0], BackgroundColor[1], BackgroundColor[2], 0);// background

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (utiliserSkybox)
		scene.drawSkybox();

	scene.drawScene();

	if (activateMirror)
	{
		drawPostProcess(true, postProcessShaderID, false);
	}

	if (activatePostProcess)
	{
		drawPostProcess(false, postProcessColorShaderID,true);
	}


	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	scene.drawMirrors(w, h, utiliserSkybox);
	drawGUI();

	// Affiche le curseur s�lectionn�
	if (typeCurseur == 0)
		SDL_ShowCursor(SDL_ENABLE);
	else
	{
		SDL_ShowCursor(SDL_DISABLE);
		drawCursor();
	}



	//swap buffer
	SDL_GL_SwapWindow(window);

	testScale += 0.05f;
}

void Renderer::resize(const int & w, const int & h)
{
	glViewport(0, 0, w, h);
}

void Renderer::mouseMotion(const unsigned int & timestamp, const unsigned int & windowID, const unsigned int & state, const int & x, const int & y, const int & xRel, const int & yRel ,Scene::KeyFlags flags)
{
	if (SDL_GetWindowID(window)==windowID && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_RIGHT))) 
	{
		scene.mouseMotion(timestamp, windowID, state, x, y, xRel, yRel);
		drawRenderer(flags);
	}
}

void Renderer::screenShot(int x, int y, int w, int h, const char * filename)
{
	unsigned int size = w * h * 4;
	unsigned char *pixels = new unsigned char[size]; // 4 bytes for RGBA
	glReadBuffer(GL_FRONT);
	glReadPixels(x, y, w, h, GL_BGRA, GL_UNSIGNED_BYTE, pixels);

	//vertical flip cause glRead goes backwards for some reason
	unsigned char *flipPixels=new unsigned char[size];
	for (int i = 0; i < w; ++i) {
		for (int j = 0; j < h; ++j) {
			for (int k = 0; k < 4; ++k) {
				flipPixels[(i + j * w) * 4 + k] = pixels[(i + (h - 1 - j) * w) * 4 + k];
			}
		}
	}

	SDL_Surface * surf = SDL_CreateRGBSurfaceFrom(flipPixels, w, h, 8 * 4, w * 4, 0, 0, 0, 0);
	SDL_SaveBMP(surf, filename);

	SDL_FreeSurface(surf);
	delete[] pixels;
	delete[] flipPixels;
	
}


Renderer::~Renderer()
{
	glDeleteProgram(primitiveShaderID);
	glDeleteBuffers(1, &primitiveShaderID);
	glDeleteProgram(modelShaderPhongID);
	glDeleteBuffers(1, &modelShaderPhongID);
	glDeleteProgram(modelShaderLambertID);
	glDeleteBuffers(1, &modelShaderLambertID);
	glDeleteProgram(modelShaderBlinnPhongID);
	glDeleteBuffers(1, &modelShaderBlinnPhongID);
	glDeleteProgram(texShaderID);
	glDeleteBuffers(1, &texShaderID);
	glDeleteProgram(postProcessShaderID);
	glDeleteBuffers(1, &postProcessShaderID);
}

void Renderer::drawGUI()
{
	int sdlWindowWidth, sdlWindowHeight;
	SDL_GetWindowSize(window, &sdlWindowWidth, &sdlWindowHeight);

	ImGui_ImplSdlGL3_NewFrame(window);

	// ********** Options de dessin **********

	ImGui::SetNextWindowPos(ImVec2(2.0f, 2.0f));
	ImGui::Begin("Options de dessin", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::ColorEdit4("Remplissage", &couleurRemplissage.r);
	ImGui::ColorEdit4("Bordures", &couleurBordure.r);
	ImGui::SliderInt("Epaisseur bordures", &epaisseurBordure, 0, 10);
	if (ImGui::Combo("Forme a dessiner", &formeADessiner, "Point\0Ligne\0Triangle\0Rectangle\0Quad\0Smiley\0Etoile\0Cube\0Pyramide\0SurfaceParam\0SurfaceTessellation\0Portail\0BezierQuad\0BezierCub\0Hermite\0bezier\0CatmullRom\0"))
		ptsDessin.clear();

	ImGui::NewLine();

	ImGui::Checkbox("Utiliser skybox", &utiliserSkybox);
	ImGui::ColorEdit3("Arriere-plan", &BackgroundColor.r);

	ImGui::NewLine();

	if (ImGui::Combo("Curseur", &typeCurseur, "Defaut\0Point\0Points\0Croix\0Triangle\0Quad\0"))
		updateCursor();

	ImGui::NewLine();

	if (ImGui::Button("Afficher texture PerlinNoise"))
		imagePerlinNoise("Resources/Image/Couleur.png");

	ImGui::SetNextWindowPos(ImVec2(2.0f, ImGui::GetCurrentWindow()->Size.y + 5.0f));
	ImGui::End();

	// ********** Importer / Exporter **********

	ImGui::Begin("Importer / Exporter", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	static char fichier[1000] = "";
	ImGui::InputText("Fichier", fichier, IM_ARRAYSIZE(fichier));

	if (ImGui::Button("Importer image"))
		importImage(string(fichier));

	if (ImGui::Button("Importer modele 3D"))
		importModel(string(fichier));
	if (ImGui::Button("Importer image pour composition"))
		imageComposition(string(fichier));

	if (ImGui::Button("Capture d'ecran"))
	{
		string fileName = fichier;
		if (fileName == "")
			fileName = "Screenshot.bmp";
		else if (fileName.substr(fileName.length() - 4) != ".bmp")
			fileName += ".bmp";

		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		screenShot(0, 0, w, h, fileName.c_str());
	}

	ImGui::SetNextWindowPos(ImVec2(2.0f, ImGui::GetCurrentWindow()->Pos.y + ImGui::GetCurrentWindow()->Size.y + 3.0f));
	ImGui::End();

	// ********** Raycasting **********

	ImGui::Begin("Raycasting", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	static char fichierRaycast[1000] = "";
	static int size[2] = { 100, 100 };
	static int rayPerPixel = 1;
	static int depth = 2;

	ImGui::InputText("Fichier", fichierRaycast, IM_ARRAYSIZE(fichier));
	ImGui::DragInt2("Largeur / Hauteur", size, 1.0f, 0, 500);
	ImGui::SliderInt("Rayons par pixel", &rayPerPixel, 1, 10);
	ImGui::SliderInt("Rebonds max", &depth, 1, 10);
	if (ImGui::Button("Generer image"))
		scene.renderRaycast(size[0], size[1], rayPerPixel, depth, fichierRaycast);
	ImGui::Text("Avertissement: Fonction bloquante.");
	ImGui::Text("Progres affiche sur la console.");

	ImGui::SetNextWindowPos(ImVec2(2.0f, ImGui::GetCurrentWindow()->Pos.y + ImGui::GetCurrentWindow()->Size.y + 3.0f));
	ImGui::End();

	// ********** Autres options **********

	ImGui::Begin("Autres options", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::Checkbox("Retroviseur", &activateMirror); 
	ImGui::Checkbox("Post-Process", &activatePostProcess);
	ImGui::Checkbox("MSAA", &MSAA);
	ImGui::Checkbox("fil de fer inverse", &activateWireframe);


	bool fog = scene.getFog();
	if (ImGui::Checkbox("Activer le brouillard", &fog))
		scene.setFog(fog);
	
	if (ImGui::Combo("Mode de projection", (int*)&projectionType, "Perspective\0Perspective inverse\0Orthographique\0"))
	{
		if (projectionType == Scene::PROJECTIONTYPE::Orthographic)
			scene.setProjection(projectionType, glm::radians(80.0f));
		else
			scene.setProjection(projectionType);
	}

	ImGui::NewLine();

	int illuminationType;
	if (currentModelShaderID == modelShaderLambertID)
		illuminationType = 0;
	else if (currentModelShaderID == modelShaderPhongID)
		illuminationType = 1;
	else
		illuminationType = 2;

	if (ImGui::Combo("Type d'illumination", &illuminationType, "Lambert\0Phong\0Blinn-Phong\0"))
	{
		switch (illuminationType)
		{
		case 0:
			currentModelShaderID = modelShaderLambertID;
			break;

		case 1:
			currentModelShaderID = modelShaderPhongID;
			break;

		case 2:
			currentModelShaderID = modelShaderBlinnPhongID;
			break;
		}
	}
	ImGui::Text("S'applique aux prochains modeles importes");

	ImGui::End();

	// ********** Graphe de sc�ne **********

	ImGui::SetNextWindowPos(ImVec2(sdlWindowWidth * 0.75f - 2.0f, 2.0f));
	ImGui::SetNextWindowSize(ImVec2(sdlWindowWidth * 0.25f, sdlWindowHeight / 4.0f));
	ImGui::Begin("Graphe de scene", (bool *)0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	if (ImGui::Button("Grouper"))
		groupNodes();
	
	ImGui::SameLine();
	if (ImGui::Button("Effacer"))
		eraseNodes();
	
	std::shared_ptr<GroupObject> root = scene.getObjects();
	bool nodeSelected = false;

	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (root->isSelected())
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
		nodeSelected = true;
	}
	
	bool nodeOpen = ImGui::TreeNodeEx("Racine", nodeFlags);
	if (ImGui::IsItemClicked())
	{
		if (nodeSelected && ImGui::GetIO().KeyCtrl)
			deselectNode(root);

		else if (nodeSelected && !ImGui::GetIO().KeyCtrl)
			deselectAllNodes();

		else if (!nodeSelected && ImGui::GetIO().KeyCtrl)
			selectNode(root, nullptr);

		else
		{
			deselectAllNodes();
			selectNode(root, nullptr);
		}
	}
	if (nodeOpen)
	{
		drawTreeRecursive(root);
		ImGui::TreePop();
	}

	if (selectedNodes.size() > 0)
		ImGui::SetNextWindowPos(ImVec2(sdlWindowWidth - transformationsWindowWidth - 2.0f, ImGui::GetCurrentWindow()->Size.y + 5.0f));

	ImGui::End();

	// ********** Transformations **********

	if (selectedNodes.size() > 0)
	{
		ImGui::Begin("Transformations", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		if (ImGui::ColorEdit4("Couleur", &currentColor.x))
			setColor();
		
		float oldShininess = currentShininess;
		if (ImGui::DragFloat("Brillance", &currentShininess, 0.1f, 0.1f, 1000.0f, "%.1F"))
		{
			currentShininess = glm::clamp(currentShininess, 0.1f, 1000.0f);
			addShininess(currentShininess - oldShininess);
		}

		glm::vec3 oldTranslation = currentTranslation;
		if (ImGui::DragFloat3("Translation", &currentTranslation.x, 0.01f, -1000.0f, 1000.0f, "%.2f"))
			addTranslation(currentTranslation - oldTranslation);

		if (useQuaternion)
		{
			glm::vec4 current(currentRotationQuat.w, currentRotationQuat.x, currentRotationQuat.y, currentRotationQuat.z);
			glm::vec4 old = current;

			if (ImGui::DragFloat4("Rotation (quat)", &current.x, 0.01f, -1000.0f, 1000.0f, "%.2f"))
			{
				currentRotationQuat = glm::quat(current.x, current.y, current.z, current.w);
				addRotation(glm::quat(current.x - old.x, current.y - old.y, current.z - old.z, current.w - old.w));
			}
		}
		else
		{
			glm::vec3 oldRotation = currentRotation;
			if (ImGui::DragFloat3("Rotation (deg)", &currentRotation.x, 0.1f, -359.9f, 359.9f, "%.1f"))
				addRotation(currentRotation - oldRotation);
		}

		glm::vec3 oldScale = currentScale;
		glm::vec3 newScale = currentScale;
		if (ImGui::DragFloat3("Echelle", &newScale.x, 0.001f, 0.001f, 1000.0f, "%.3f"))
		{
			if (proportionalResizing)
			{
				float diff = (newScale.x / currentScale.x) * (newScale.y / currentScale.y) * (newScale.z / currentScale.z);
				currentScale = glm::vec3(currentScale.x * diff, currentScale.y * diff, currentScale.z * diff);
			}
			else
				currentScale = newScale;

			addScale(currentScale - oldScale);
		}

		ImGui::Checkbox("Redimensionnement proportionnel", &proportionalResizing);
		ImGui::Checkbox("Utiliser les quaternions", &useQuaternion);

		if (selectedNodes.size() == 1&& isCastable<ParametricSurfaceObject,AbstractObject>(selectedNodes[0].first.get()) )
		{
			ImGui::DragFloat4("Parameters1", &currentMat4[0][0], 0.01f, -1000.0f, 1000.0f, "%.2f");
			ImGui::DragFloat4("2", &currentMat4[1][0], 0.01f, -1000.0f, 1000.0f, "%.2f");
			ImGui::DragFloat4("3", &currentMat4[2][0], 0.01f, -1000.0f, 1000.0f, "%.2f");
			ImGui::DragFloat4("4", &currentMat4[3][0], 0.01f, -1000.0f, 1000.0f, "%.2f");
			ParametricSurfaceObject* object = getCasted<ParametricSurfaceObject, AbstractObject>(selectedNodes[0].first.get());
			object->setMatrix(currentMat4);
		}

		transformationsWindowWidth = ImGui::GetCurrentWindow()->Size.x;
		ImGui::End();
	}

	// ********** �chantillonnage d�image  **********

	ImGui::SetNextWindowPos(ImVec2(2.0f, sdlWindowHeight - samplingWindowHeight - 2.0f));
	ImGui::Begin("Echantillonnage d'image", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	static char imageBase[1000] = "";
	static char imageEchantillon[1000] = "";

	ImGui::InputText("Image de base", imageBase, IM_ARRAYSIZE(imageBase));
	ImGui::InputText("Image d'echantillonnage", imageEchantillon, IM_ARRAYSIZE(imageEchantillon));

	ImGui::NewLine();

	ImGui::SliderInt("Pourcentage de l'image a incorporer", &pourcentageImage, 0, 100);
	ImGui::Combo("Position de depart", &postionEchantillonnage, "Haut-Gauche\0Haut-Milieu\0Milieu-Gauche\0Milieu-Milieu\0");

	ImGui::NewLine();

	if (ImGui::Button("Commencer Echantillonnage"))
		echantillonnageImage(imageBase,imageEchantillon);

	samplingWindowHeight = ImGui::GetCurrentWindow()->Size.y;
	ImGui::End();

	// ********** Lumi�res **********

	ImGui::SetNextWindowPos(ImVec2(sdlWindowWidth - lightsWindowSize.x - 2.0f, sdlWindowHeight - lightsWindowSize.y - 2.0f));
	ImGui::Begin("Lumieres", (bool *)0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::Combo("Type", &lightType, "Directionnelle\0Ponctuelle\0Projecteur\0");
	ImGui::ColorEdit3("Couleur ambiante", &lightAmbientColor.r);
	ImGui::SliderFloat("Intensite ambiante", &lightAmbientIntensity, 0.0f, 1.0f);
	ImGui::ColorEdit3("Couleur diffuse", &lightDiffuseColor.r);
	ImGui::SliderFloat("Intensite diffuse", &lightDiffuseIntensity, 0.0f, 1.0f);
	ImGui::ColorEdit3("Couleur speculaire", &lightSpecularColor.r);
	ImGui::SliderFloat("Intensite speculaire", &lightSpecularIntensity, 0.0f, 1.0f);
	ImGui::DragFloat3("volume", &lightVolume.x, 1.0f, -359.9f, 359.9f, "%.1f");
	ImGui::Checkbox("Volumetrique", &lightVolumetric);

	if (lightType != 0)
		ImGui::SliderFloat("Attenuation", &lightAttenuation, 0.0f, 1.0f);

	if (lightType != 1)
		ImGui::DragFloat3("Direction", &lightDirection.x, 0.01f, -1000.0f, 1000.0f, "%.2f");

	if (lightType != 0)
		ImGui::DragFloat3("Position", &lightPosition.x, 0.01f, -1000.0f, 1000.0f, "%.2f");

	if (lightType == 2)
		ImGui::SliderFloat("Angle cone", &lightConeAngle, 0.01f, 179.9f, "%.2f");

	if (ImGui::Button("Ajouter une lumiere"))
	{
		std::shared_ptr<LightObject> light = std::make_shared<LightObject>();
		light->setLight(
			lightType,
			lightAmbientColor,
			lightAmbientIntensity,
			lightDiffuseColor,
			lightDiffuseIntensity,
			lightSpecularColor,
			lightSpecularIntensity,
			lightAttenuation,
			lightDirection,
			lightPosition,
			lightConeAngle,
			lightVolumetric,
			lightVolume);

		scene.addObject(light);
		scene.setupLight();
	}

	lightsWindowSize = ImVec2(ImGui::GetCurrentWindow()->Size.x, ImGui::GetCurrentWindow()->Size.y);
	ImGui::End();

	// Render
	ImGui::Render();
	ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::drawTreeRecursive(std::shared_ptr<GroupObject> objects)
{
	for (unsigned int i = 0; i < objects->size(); ++i)
	{
		std::shared_ptr<AbstractObject> obj = objects->getObjectAt(i);
		std::shared_ptr<GroupObject> group = castToGroupObject(obj);

		bool isGroup = group != nullptr;
		bool nodeSelected = false;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
		if (!isGroup)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (obj->isSelected())
		{
			flags |= ImGuiTreeNodeFlags_Selected;
			nodeSelected = true;
		}

		bool nodeOpen = ImGui::TreeNodeEx(obj.get(), flags, obj->getName().c_str());
		if (ImGui::IsItemClicked())
		{
			if (nodeSelected && ImGui::GetIO().KeyCtrl)
				deselectNode(obj);

			else if (nodeSelected && !ImGui::GetIO().KeyCtrl)
				deselectAllNodes();

			else if (!nodeSelected && ImGui::GetIO().KeyCtrl)
				selectNode(obj, objects);

			else
			{
				deselectAllNodes();
				selectNode(obj, objects);
			}
		}
		if (nodeOpen && isGroup)
		{
			drawTreeRecursive(group);
			ImGui::TreePop();
		}
	}
}

void Renderer::drawCursor()
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	SDL_GetMouseState(&x, &y);
	SDL_GetWindowSize(window, &w, &h);

	// Conversion en coordonn�es centr�es sur la fen�tre
	float halfW = w / 2.0f;
	float halfH = h / 2.0f;
	float glX = x / halfW - 1.0f;
	float glY = -y / halfH + 1.0f;

	std::vector<glm::vec3> positions;

	switch (typeCurseur)
	{
	case 1: // Point
		positions.push_back(glm::vec3(glX, glY, 0.0f));
		break;
		
	case 2: // Points
		positions.push_back(glm::vec3(glX, glY, 0.0f));
		positions.push_back(glm::vec3(glX, glY + 10.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX, glY - 10.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX - 10.0f / halfW, glY, 0.0f));
		positions.push_back(glm::vec3(glX + 10.0f / halfW, glY, 0.0f));
		break;

	case 3: // Croix
		positions.push_back(glm::vec3(glX, glY + 10.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX, glY - 10.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX - 10.0f / halfW, glY, 0.0f));
		positions.push_back(glm::vec3(glX + 10.0f / halfW, glY, 0.0f));
		break;

	case 4: // Triangle
		positions.push_back(glm::vec3(glX, glY, 0.0f));
		positions.push_back(glm::vec3(glX, glY - 15.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX + 15.0f / halfW, glY, 0.0f));
		break;

	case 5: // Quad
		positions.push_back(glm::vec3(glX, glY, 0.0f));
		positions.push_back(glm::vec3(glX, glY - 15.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX + 5.0f / halfW, glY - 10.0f / halfH, 0.0f));
		positions.push_back(glm::vec3(glX + 10.0f / halfW, glY - 10.0f / halfH, 0.0f));
		break;
	}

	curseur.setVertices(positions);
	curseur.Draw();
}



void Renderer::drawPostProcess(bool mirror, GLuint program,bool fullScreen)
{
	
	int w, h;
	SDL_GetWindowSize(window, &w, &h);


	GLuint fbo, fbo_texture, rbo_depth, vbo_fbo_vertices,vao;
	GLfloat fbo_vertices[8];
	if (fullScreen)
	{
		fbo_vertices[0] = -1; fbo_vertices[1] = -1;
		fbo_vertices[2] = 1; fbo_vertices[3] = -1;
		fbo_vertices[4] = -1; fbo_vertices[5] = 1;
		fbo_vertices[6] = 1; fbo_vertices[7] = 1;

	}
	else 
	{
		fbo_vertices[0] = 0; fbo_vertices[1] = -1;
		fbo_vertices[2] = 1; fbo_vertices[3] = -1;
		fbo_vertices[4] = 0; fbo_vertices[5] = 0;
		fbo_vertices[6] = 1; fbo_vertices[7] = 0;
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo_fbo_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &fbo_texture);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	/* Depth buffer */
	glGenRenderbuffers(1, &rbo_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	/* Framebuffer to link everything together */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);
	GLenum status;
	if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
		cout << "framebuffer error" << endl;
		return;
	}

	glClearColor(BackgroundColor[0], BackgroundColor[1], BackgroundColor[2], 0);// background
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mirror)
	{
		scene.addYaw(180);
	}

	if (utiliserSkybox)
		scene.drawSkybox();

	scene.drawScene();

	if (mirror)
	{
		scene.addYaw(-180);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	

	//Quad to draw on
	glUseProgram(program);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glBindVertexArray(vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fbo_texture);
	glUniform1i(glGetUniformLocation(fbo_texture, "text"), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);


	//clean buffers
	glDeleteRenderbuffers(1, &rbo_depth);
	glDeleteTextures(1, &fbo_texture);
	glDeleteFramebuffers(1, &fbo);
	glDeleteBuffers(1, &vbo_fbo_vertices);

}

void Renderer::updateCursor()
{
	switch (typeCurseur)
	{
	case 1: // Point
	case 2: // Points
		curseur.setEpaisseurBordure(3);
		curseur.setTypePrimitive(GL_POINTS);
		break;

	case 3: // Croix
		curseur.setEpaisseurBordure(2);
		curseur.setTypePrimitive(GL_LINES);
		break;

	case 4: // Triangle
		curseur.setEpaisseurBordure(1);
		curseur.setTypePrimitive(GL_TRIANGLES);
		break;

	case 5: // Quad
		curseur.setEpaisseurBordure(1);
		curseur.setTypePrimitive(GL_TRIANGLE_FAN);
		break;
	}
}

void Renderer::importImage(string fichier)
{
	std::ifstream f(fichier);
	if (!f.good())
	{
		cout << "Fichier inexistant" << endl;
		return;
	}
	f.close();

	scene.addObject(std::make_shared<QuadObject>(fichier));
	std::shared_ptr<GroupObject> objects= scene.getObjects();
	objects->getObjectAt(objects->size()-1)->Create(texShaderID);
	scene.setupLight();
}

void Renderer::importModel(string file)
{
	std::ifstream f(file);
	if (!f.good())
	{
		cout << "File not found" << endl;
		return;
	}
	f.close();

	ModelObject object;
	object.setModelToCreate(file);
	object.Create(currentModelShaderID);
	scene.addObject(std::make_shared<ModelObject>(object));
	//Resources/megalodon/megalodon.FBX
	scene.setupLight();
}
void Renderer::imagePerlinNoise(string fichier)
{
	std::ifstream f(fichier);
	if (!f.good())
	{
		cout << "Fichier inexistant" << endl;
		return;
	}
	f.close();

	scene.addObject(std::make_shared<QuadObject>(fichier,"perlinNoise"));
	std::shared_ptr<GroupObject> objects= scene.getObjects();
	objects->getObjectAt(objects->size()-1)->Create(texShaderID);
}
void Renderer::imageComposition(string fichier)
{
	std::ifstream f(fichier);
	if (!f.good())
	{
		cout << "Fichier inexistant" << endl;
		return;
	}
	f.close();

	scene.addObject(std::make_shared<QuadObject>(fichier, "composition"));
	std::shared_ptr<GroupObject> objects = scene.getObjects();
	objects->getObjectAt(objects->size() - 1)->Create(texShaderID);
}

void Renderer::ajouterPtDessin(int x, int y)
{
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	float glX = x / (w / 2.0f) - 1.0f;
	float glY = -y / (h / 2.0f) + 1.0f;

	ptsDessin.push_back(glm::vec3(glX, glY, 0.0f));

	// Type de primitive
	GLenum typePrimitive = -1;
	string name = "Primitive";

	switch (formeADessiner)
	{
	case 0: // Point
		typePrimitive = GL_POINTS;
		name = "Point";
		break;

	case 1: // Ligne
		if (ptsDessin.size() >= 2)
		{
			typePrimitive = GL_LINES;
			name = "Ligne";
		}
		break;

	case 2: // Triangle
		if (ptsDessin.size() >= 3)
		{
			typePrimitive = GL_TRIANGLES;
			name = "Triangle";
		}
		break;

	case 3: // Rectangle
		if (ptsDessin.size() >= 2)
		{
			typePrimitive = GL_TRIANGLE_FAN;
			name = "Rectangle";
		}
		break;

	case 4: // Quad
		if (ptsDessin.size() >= 4)
		{
			typePrimitive = GL_TRIANGLE_FAN;
			name = "Quad";
		}
		break;

	case 5: // Smiley
		if (ptsDessin.size() >= 2)
			ajouterSmiley();
		break;

	case 6: // �toile
		if (ptsDessin.size() >= 2)
			ajouterEtoile();
		break;

	case 7://cube
		addCube();
		break;

	case 8://pyramid
		addSBPyramid();
		break;
	case 9://surface parametrique
		addParametricSurface();
		break;
	case 10://surface tessellation
		addSurfaceTessellation();
		break;
	case 11://Portail
		addMirror();
		break;
	case 12://bezier quad
		if (ptsDessin.size() >= 3)
			addParametricCurve(ParametricCurveObject::BezierQuadratic);
		break;
	case 13://bezier cubic
		if (ptsDessin.size() >= 4)
			addParametricCurve(ParametricCurveObject::BezierCubic);
		break;
	case 14://hermite
		if (ptsDessin.size() >= 4)
			addParametricCurve(ParametricCurveObject::Hermite);
		break;
	case 15://bezier
		if (ptsDessin.size() >= 2)//change that 
			addParametricCurve(ParametricCurveObject::Bezier);
		break;
	case 16://catmullrom
		if (ptsDessin.size() >= 4)
			addParametricCurve(ParametricCurveObject::CatmullRom);
		break;
	//BezierQuad\0BezierCub\0Hermite\0bezier\0CatmullRom\0

	}

	// Ajout de primitive
	if (typePrimitive != -1)
	{
		std::shared_ptr<PrimitiveObject> primitive = std::make_shared<PrimitiveObject>();
		primitive->Create(primitiveShaderID, name);
		primitive->setCouleurBordure(couleurBordure);
		primitive->setCouleurRemplissage(couleurRemplissage);
		primitive->setEpaisseurBordure(epaisseurBordure);
		primitive->setTypePrimitive(typePrimitive);

		// Lorsqu'on dessine un rectangle, on donne les coordonn�es de 2 sommets oppos�s
		// On doit donc ajouter 2 sommets au dessin
		if (formeADessiner == 3)
			ptsDessin = { ptsDessin[0], glm::vec3(ptsDessin[0].x, ptsDessin[1].y, 0.0f), ptsDessin[1], glm::vec3(ptsDessin[1].x, ptsDessin[0].y, 0.0f) };

		primitive->setVertices(ptsDessin);

		scene.addObject(primitive);

		ptsDessin.clear();
	}
}

void Renderer::addCube() 
{
	scene.addObject(make_shared<CubeObject>());
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->Create(GPShaderID);
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->setColor(couleurRemplissage);
	ptsDessin.clear();
}

void Renderer::addSBPyramid()
{
	scene.addObject(make_shared<SBPyramidObject>());
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->Create(GPShaderID);
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->setColor(couleurRemplissage);
	ptsDessin.clear();
}

void Renderer::addParametricSurface()
{
	scene.addObject(make_shared<ParametricSurfaceObject>());
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->Create(GPShaderID);
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->setColor(couleurRemplissage);
	ptsDessin.clear();
}

void Renderer::addSurfaceTessellation()
{
	scene.addObject(make_shared<TessellationQuad>("Resources/plancher2.png"));
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->Create(tessellationShaderID);
	ptsDessin.clear();
}

void Renderer::addMirror()
{
	scene.addObject(make_shared<MirrorObject>());
	scene.getObjects()->getCastedObjectAt<MirrorObject>(scene.getObjects()->size() - 1)->setInverted(false, true);
	scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->Create(texShaderID);	
	scene.setupMirrors();
	ptsDessin.clear();
}

void Renderer::addParametricCurve(ParametricCurveObject::PARAMETRICTYPE type)
{
	if (type != ParametricCurveObject::Bezier || ptsDessin.size() <= 2) {
		scene.addObject(make_shared<ParametricCurveObject>());
		scene.getObjects()->getObjectAt(scene.getObjects()->size() - 1)->Create(primitiveShaderID);
		scene.getObjects()->getCastedObjectAt<ParametricCurveObject>(scene.getObjects()->size() - 1)->setParametricType(type);
		scene.getObjects()->getCastedObjectAt<ParametricCurveObject>(scene.getObjects()->size() - 1)->setNumLines(100);
	}
	scene.getObjects()->getCastedObjectAt<ParametricCurveObject>(scene.getObjects()->size() - 1)->setVertices(ptsDessin);
	scene.getObjects()->getCastedObjectAt<ParametricCurveObject>(scene.getObjects()->size() - 1)->setColor(couleurRemplissage);
	scene.getObjects()->getCastedObjectAt<ParametricCurveObject>(scene.getObjects()->size() - 1)->setEdgeColor(couleurRemplissage);
	scene.getObjects()->getCastedObjectAt<ParametricCurveObject>(scene.getObjects()->size() - 1)->setEdgeSize(5);
	if(type!= ParametricCurveObject::Bezier)
		ptsDessin.clear();
}

void Renderer::ajouterSmiley()
{
	const int nbPrimitives = 4;
	std::shared_ptr<PrimitiveObject> primitives[nbPrimitives];
	
	// Initialisation
	for (int i = 0; i < nbPrimitives; ++i)
		primitives[i] = std::make_shared<PrimitiveObject>();

	primitives[0]->Create(primitiveShaderID, "Smiley");
	primitives[1]->Create(primitiveShaderID, "Oeil gauche");
	primitives[2]->Create(primitiveShaderID, "Oeil droit");
	primitives[3]->Create(primitiveShaderID, "Sourire");

	for (int i = 0; i < nbPrimitives; i++)
	{
		primitives[i]->setCouleurBordure(couleurBordure);

		if (i == 0)
			primitives[i]->setCouleurRemplissage(couleurRemplissage);
		else
			primitives[i]->setCouleurRemplissage(couleurBordure);

		primitives[i]->setEpaisseurBordure(epaisseurBordure);
		primitives[i]->setTypePrimitive(GL_TRIANGLE_FAN);
	}

	glm::vec3 topLeft = { min(ptsDessin[0].x, ptsDessin[1].x), max(ptsDessin[0].y, ptsDessin[1].y), 0.0f };
	glm::vec3 botRight = { max(ptsDessin[0].x, ptsDessin[1].x), min(ptsDessin[0].y, ptsDessin[1].y), 0.0f };
	float w = botRight.x - topLeft.x;
	float h = topLeft.y - botRight.y;

	std::vector<glm::vec3> vertices;

	// Carr� principal
	vertices = { topLeft, glm::vec3(topLeft.x, botRight.y, 0.0f), botRight, glm::vec3(botRight.x, topLeft.y, 0.0f) };
	primitives[0]->setVertices(vertices);

	// Oeil gauche
	vertices = { 
		glm::vec3(topLeft.x + 0.125f * w, topLeft.y - 0.125f * h, 0.0f),
		glm::vec3(topLeft.x + 0.125f * w, topLeft.y - 0.375f * h, 0.0f),
		glm::vec3(topLeft.x + 0.375f * w, topLeft.y - 0.375f * h, 0.0f),
		glm::vec3(topLeft.x + 0.375f * w, topLeft.y - 0.125f * h, 0.0f)
	};
	primitives[1]->setVertices(vertices);

	// Oeil droit
	vertices = {
		glm::vec3(botRight.x - 0.375f * w, topLeft.y - 0.125f * h, 0.0f),
		glm::vec3(botRight.x - 0.375f * w, topLeft.y - 0.375f * h, 0.0f),
		glm::vec3(botRight.x - 0.125f * w, topLeft.y - 0.375f * h, 0.0f),
		glm::vec3(botRight.x - 0.125f * w, topLeft.y - 0.125f * h, 0.0f)
	};
	primitives[2]->setVertices(vertices);

	// Sourire
	vertices = {
		glm::vec3(topLeft.x + 0.125f * w, botRight.y + 0.375f * h, 0.0f),
		glm::vec3(topLeft.x + 0.25f * w, botRight.y + 0.125f * h, 0.0f),
		glm::vec3(botRight.x - 0.25f * w, botRight.y + 0.125f * h, 0.0f),
		glm::vec3(botRight.x - 0.125f * w, botRight.y + 0.375f * h, 0.0f)
	};
	primitives[3]->setVertices(vertices);

	std::shared_ptr<GroupObject> smiley = std::make_shared<GroupObject>();
	for (int i = 0; i < nbPrimitives; i++)
		smiley->addObject(primitives[i]);

	scene.addObject(smiley);

	ptsDessin.clear();
}

void Renderer::ajouterEtoile()
{
	std::shared_ptr<PrimitiveObject> primitive = std::make_shared<PrimitiveObject>();
	primitive->Create(primitiveShaderID, "Etoile");
	primitive->setCouleurBordure(couleurBordure);
	primitive->setEpaisseurBordure(epaisseurBordure);
	primitive->setTypePrimitive(GL_LINES);
	
	glm::vec3 topLeft = { min(ptsDessin[0].x, ptsDessin[1].x), max(ptsDessin[0].y, ptsDessin[1].y), 0.0f };
	glm::vec3 botRight = { max(ptsDessin[0].x, ptsDessin[1].x), min(ptsDessin[0].y, ptsDessin[1].y), 0.0f };
	float w = botRight.x - topLeft.x;
	float h = topLeft.y - botRight.y;

	std::vector<glm::vec3> vertices;
	vertices.push_back(topLeft);
	vertices.push_back(botRight);
	vertices.push_back(glm::vec3(topLeft.x + 0.5f * w, topLeft.y, 0.0f));
	vertices.push_back(glm::vec3(topLeft.x + 0.5f * w, botRight.y, 0.0f));
	vertices.push_back(glm::vec3(botRight.x, topLeft.y, 0.0f));
	vertices.push_back(glm::vec3(topLeft.x, botRight.y, 0.0f));
	vertices.push_back(glm::vec3(botRight.x, botRight.y + 0.5f * h, 0.0f));
	vertices.push_back(glm::vec3(topLeft.x, botRight.y + 0.5f * h, 0.0f));

	primitive->setVertices(vertices);

	scene.addObject(primitive);

	ptsDessin.clear();
}

void Renderer::deselectAllNodes()
{
	for (auto pair : selectedNodes)
		pair.first->setSelected(false);

	selectedNodes.clear();

	updateTransformations();
}

void Renderer::deselectNode(std::shared_ptr<AbstractObject> obj)
{
	obj->setSelected(false);

	for (auto it = selectedNodes.begin(); it != selectedNodes.end(); ++it)
	{
		if (it->first == obj)
		{
			selectedNodes.erase(it);
			return;
		}
	}

	updateTransformations();
}

void Renderer::selectNode(std::shared_ptr<AbstractObject> obj, std::shared_ptr<GroupObject> parent)
{
	obj->setSelected(true);
	selectedNodes.push_back(std::make_pair(obj, parent));
	updateTransformations();
}

std::shared_ptr<GroupObject> Renderer::castToGroupObject(std::shared_ptr<AbstractObject> obj)
{
	if (std::shared_ptr<GroupObject> casted = dynamic_pointer_cast<GroupObject>(obj))
	{
		return casted;
	}
	else
	{
		return nullptr;
	}
}

void Renderer::eraseNodes()
{
	for (auto pair : selectedNodes)
	{
		pair.first->setSelected(false);
		if (pair.second != nullptr)
			pair.second->deleteObject(pair.first);
	}
	selectedNodes.clear();
	scene.setupLight();
	scene.setupMirrors();
}

void Renderer::groupNodes()
{
	if (selectedNodes.size() == 0)
		return;

	// Add all selected objects to a child group object
	GroupObject childGroup;
	for (auto pair : selectedNodes)
	{
		if (pair.second == nullptr)
			return;
		childGroup.addObject(pair.first);
	}

	// Add child group object to the first selected object's parent
	selectedNodes[0].second->addObject(std::make_shared<GroupObject>(childGroup));

	// Erase original nodes
	eraseNodes();
}

void Renderer::setColor()
{
	for (auto pair : selectedNodes)
		pair.first->setColor(currentColor);
}

void Renderer::addShininess(const float &v)
{
	for (auto pair : selectedNodes)
		pair.first->addShininess(v);
}

void Renderer::addTranslation(const glm::vec3 &v)
{
	for (auto pair : selectedNodes)
		pair.first->addPosition(v);
}

void Renderer::addRotation(const glm::vec3 &v)
{
	for (auto pair : selectedNodes)
		pair.first->addRotationDegree(v);
}

void Renderer::addRotation(const glm::quat &q)
{
	for (auto pair : selectedNodes)
		pair.first->addRotationQuaternion(q);
}

void Renderer::addScale(const glm::vec3 &v)
{
	for (auto pair : selectedNodes)
		pair.first->addScale(v);
}

void Renderer::updateTransformations()
{
	if (selectedNodes.size() == 1) // Un seul s�lectionn�
	{
		std::shared_ptr<AbstractObject> obj = selectedNodes[0].first;

		currentColor = obj->getColor();
		currentShininess = obj->getShininess();
		currentRotation = obj->getRotationDegree();
		currentRotationQuat = obj->getRotationQuaternion();
		currentTranslation = obj->getPosition();
		currentScale = obj->getScale();
		if ( isCastable<ParametricSurfaceObject, AbstractObject>(selectedNodes[0].first.get()))
		{
			ParametricSurfaceObject* object = getCasted<ParametricSurfaceObject, AbstractObject>(selectedNodes[0].first.get());
			currentMat4= object->getMatrix();
		}
	}
	else // Aucun ou plusieurs s�lectionn�s
	{
		currentColor = glm::vec4(0, 0, 0, 1);
		currentShininess = 50.0f;
		currentRotation = glm::vec3(0,0,0);
		currentRotationQuat = glm::quat(1,0,0,0);
		currentTranslation = glm::vec3(0, 0, 0);
		currentScale = glm::vec3(1, 1, 1);
	}
}

void Renderer::echantillonnageImage(string imageBase,string imageEchantillon)
{
	std::ifstream f(imageBase);
	if (!f.good())
	{
		cout << "Fichier pour image de base inexistant" << endl;
		return;
	}
	f.close();

	std::ifstream f2(imageEchantillon);
	if (!f2.good())
	{
		cout << "Fichier pour image d'echantillonnage inexistant" << endl;
		return;
	}
	f2.close();

	string s1 = to_string(postionEchantillonnage) + "|";
	string s2 = to_string(pourcentageImage) + "|";
	imageEchantillon = s1 + imageEchantillon;
	imageEchantillon = s2 + imageEchantillon;

	scene.addObject(std::make_shared<QuadObject>(imageBase, imageEchantillon));
	std::shared_ptr<GroupObject> objects = scene.getObjects();
	objects->getObjectAt(objects->size() - 1)->Create(texShaderID);
}
