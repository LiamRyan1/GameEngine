#include <GL/glew.h>
#include "../include/Rendering/Renderer.h"
#include "../include./Rendering/Texture.h"
#include "../include/Rendering/MeshFactory.h"
#include <iostream>
#include <fstream>   
#include <sstream> 
#include "../include/Rendering/Camera.h"
#include "../include/Scene/Transform.h"
#include "../include/Scene/GameObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

Renderer::Renderer() : shadowMap(4096, 4096), skyboxEnabled(false), debugPhysicsEnabled(false) {
	mainLight.setDirection(glm::vec3(0.3f, -1.0f, 0.5f));
	mainLight.setIntensity(0.8f);
}

Renderer::~Renderer() {
}

void Renderer::initialize() {
	// Create shader programs
	shaderManager.createProgram("shaders/basic.vert", "shaders/basic.frag", "main");
	shaderManager.createProgram("shaders/shadow_depth.vert", "shaders/shadow_depth.frag", "shadow");

	shadowMap.initialize();

	cubeMesh = MeshFactory::createCube();
	sphereMesh = MeshFactory::createSphere();
	cylinderMesh = MeshFactory::createCylinder();
}

bool Renderer::loadSkybox(const std::vector<std::string>& faces) {
	if (skybox.loadCubemap(faces)) {
		skyboxEnabled = true;
		return true;
	}
	return false;
}

void Renderer::renderShadowPass(
	const Camera& camera,
	const std::vector<std::unique_ptr<GameObject>>& objects)
{
	// Calculate light space matrix
	glm::vec3 sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);  // Could be dynamic based on objects
	float sceneRadius = 50.0f;  // Could be calculated from scene bounds
	glm::mat4 lightSpaceMatrix = mainLight.getLightSpaceMatrix(sceneCenter, sceneRadius);

	// Bind shadow map for writing
	shadowMap.bindForWriting();

	// Use shadow shader
	unsigned int shadowShader = shaderManager.getProgram("shadow");
	glUseProgram(shadowShader);

	int lightSpaceLoc = glGetUniformLocation(shadowShader, "lightSpaceMatrix");
	int modelLoc = glGetUniformLocation(shadowShader, "model");

	glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

	// Render all objects (only their depth)
	for (const auto& obj : objects) {
		glm::mat4 model = Transform::model(
			obj->getPosition(),
			obj->getRotation(),
			obj->getScale()
		);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

		// Use the object's render mesh
		Mesh* mesh = obj->getRender().getRenderMesh();
		if (!mesh) mesh = &cubeMesh;

		mesh->draw();
	}

	// Unbind shadow map
	shadowMap.unbind();
}

void Renderer::drawGameObject(const GameObject& obj, int modelLoc, int colorLoc) {
	glm::mat4 model = Transform::model(
		obj.getPosition(),
		obj.getRotation(),
		obj.getScale()
	);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	// Use render component's mesh
	Mesh* mesh = obj.getRender().getRenderMesh();
	if (!mesh) mesh = &cubeMesh;  // Fallback safety

	glm::vec3 color(1.0f, 0.5f, 0.2f);

	bool hasTexture = !obj.getTexturePath().empty();
	Texture* texture = nullptr;

	if (hasTexture) {
		texture = textureManager.loadTexture(obj.getTexturePath());
	}

	unsigned int mainShader = shaderManager.getProgram("main");
	int useTexLoc = glGetUniformLocation(mainShader, "useTexture");
	int texLoc = glGetUniformLocation(mainShader, "textureSampler");;

	if (texture) {
		texture->bind(0);
		glUniform1i(texLoc, 0);
		glUniform1i(useTexLoc, 1);
	}
	else {
		glUniform1i(useTexLoc, 0);
		glUniform3f(colorLoc, color.r, color.g, color.b);
	}

	if (texture) {
		texture->bind(0);
		glUniform1i(texLoc, 0);
		glUniform1i(useTexLoc, 1);

		// IMPORTANT: prevent shader’s "edge black" early-return from triggering
		glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
	}
	else {
		glUniform1i(useTexLoc, 0);
		glUniform3f(colorLoc, color.r, color.g, color.b);
	}


	mesh->draw();

	if (texture) {
		texture->unbind();
	}
}

void Renderer::draw(int windowWidth,
	int windowHeight,
	const Camera& camera,
	const std::vector<std::unique_ptr<GameObject>>& objects,
	const GameObject* primarySelection,
	const std::vector<GameObject*>& selectedObjects) {
	if (windowHeight == 0)
		return;

	// SHADOW PASS - Render from light's perspective
	renderShadowPass(camera, objects);

	// MAIN PASS - Render scene normally
	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Draw skybox first
	float aspectRatio = (float)windowWidth / (float)windowHeight;
	glm::mat4 view = camera.getViewMatrix();
	glm::mat4 projection = camera.getProjectionMatrix(aspectRatio);

	if (skyboxEnabled) {
		skybox.draw(view, projection);
	}

	unsigned int mainShader = shaderManager.getProgram("main");
	glUseProgram(mainShader);

	int modelLoc = glGetUniformLocation(mainShader, "model");
	int viewLoc = glGetUniformLocation(mainShader, "view");
	int projectionLoc = glGetUniformLocation(mainShader, "projection");
	int colorLoc = glGetUniformLocation(mainShader, "objectColor");
	int lightDirLoc = glGetUniformLocation(mainShader, "lightDir");
	int viewPosLoc = glGetUniformLocation(mainShader, "viewPos");
	int lightColorLoc = glGetUniformLocation(mainShader, "lightColor");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);

	// Set lighting uniforms from DirectionalLight
	glm::vec3 cameraPos = camera.getPosition();

	glUniform3fv(lightDirLoc, 1, &mainLight.getDirection()[0]);
	glUniform3fv(viewPosLoc, 1, &cameraPos[0]);
	glUniform3fv(lightColorLoc, 1, &mainLight.getFinalColor()[0]);


	// Calculate and pass light space matrix
	glm::vec3 sceneCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	float sceneRadius = 50.0f;
	glm::mat4 lightSpaceMatrix = mainLight.getLightSpaceMatrix(sceneCenter, sceneRadius);
	int lightSpaceMatrixLoc = glGetUniformLocation(mainShader, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

	// Bind shadow map texture to texture unit 1
	shadowMap.bindForReading(1);
	int shadowMapLoc = glGetUniformLocation(mainShader, "shadowMap");
	glUniform1i(shadowMapLoc, 1);


	for (const auto& obj : objects)
	{
		bool isSelected =
			std::find(selectedObjects.begin(),
				selectedObjects.end(),
				obj.get()) != selectedObjects.end();

		glUniform1i(glGetUniformLocation(mainShader, "uIsSelected"),
			isSelected ? 1 : 0);

		glUniform3f(glGetUniformLocation(mainShader, "uHighlightColor"),
			0.0f, 1.0f, 1.0f); // cyan

		glUniform1f(glGetUniformLocation(mainShader, "uHighlightStrength"),
			0.6f);

		drawGameObject(*obj, modelLoc, colorLoc);

		// Outline ONLY for primary selection
		if (primarySelection && obj.get() == primarySelection)
		{
			drawOutlineOnly(*obj, modelLoc, colorLoc);
		}

		if (debugPhysicsEnabled)
		{
			drawDebugCollisionShape(*obj, modelLoc, colorLoc);
		}
	}
}

// Draws a black wire overlay on top of the object (if mesh has edge indices).
// Does NOT change your friend's base draw code.
void Renderer::drawOutlineOnly(const GameObject& obj, int modelLoc, int colorLoc)
{
	// Build model matrix
	glm::mat4 model = Transform::model(obj.getPosition(), obj.getRotation(), obj.getScale());
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	Mesh* mesh = obj.getRender().getRenderMesh();
	if (!mesh) mesh = &cubeMesh;

	// Force "edge shader path" to black using your existing fragment test
	unsigned int mainShader = shaderManager.getProgram("main");
	int useTexLoc = glGetUniformLocation(mainShader, "useTexture");
	glUniform1i(useTexLoc, 0);
	glUniform3f(colorLoc, 0.0f, 0.0f, 0.0f);

	// Render as wireframe overlay
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-0.5f, -0.5f);
	glLineWidth(2.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	mesh->draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_POLYGON_OFFSET_LINE);
}

void Renderer::drawDebugCollisionShape(const GameObject& obj, int modelLoc, int colorLoc) {
	// Build model matrix
	glm::mat4 model = Transform::model(
		obj.getPosition(),
		obj.getRotation(),
		obj.getScale()
	);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	// Pick mesh
	Mesh* mesh = nullptr;
	switch (obj.getShapeType()) {
	case ShapeType::CUBE:
		mesh = &cubeMesh;
		break;
	case ShapeType::SPHERE:
		mesh = &sphereMesh;
		break;
	case ShapeType::CAPSULE:
		mesh = &cylinderMesh;
		break;
	}

	if (!mesh) return;

	// Disable depth test so wireframe is always visible
	glDisable(GL_DEPTH_TEST);

	// Set light to super bright
	unsigned int mainShader = shaderManager.getProgram("main");
	int lightColorLoc = glGetUniformLocation(mainShader, "lightColor");
	glUniform3f(lightColorLoc, 10.0f, 10.0f, 10.0f);  // Super bright light


	// Draw as bright cyan wireframe
	int useTexLoc = glGetUniformLocation(mainShader, "useTexture");
	glUniform1i(useTexLoc, 0);
	glUniform3f(colorLoc, 0.0f, 1.0f, 1.0f);  // Cyan

	glLineWidth(1.5f);

	// Draw using polygon mode instead of edge indices
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Wireframe mode
	mesh->draw();  // Use regular draw (faces), but in LINE mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Back to normal

	// Restore light color
	glUniform3fv(lightColorLoc, 1, &mainLight.getFinalColor()[0]);

	// Re-enable depth test
	glEnable(GL_DEPTH_TEST);
}

void Renderer::cleanup() {
	std::cout << "Cleaning up renderer..." << std::endl;
	cubeMesh.cleanup();
	skybox.cleanup();
	shadowMap.cleanup();

	textureManager.cleanup();
	shaderManager.cleanup();
	std::cout << "Renderer cleaned up" << std::endl;
}