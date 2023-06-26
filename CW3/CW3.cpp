#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>


#include <iostream>
#include <thread>
#include <future>
#include <cmath>
#include <algorithm>

#include "headers/error.h"
#include "headers/shader.h"
#include "headers/ftcarema.h"
#include "headers/objparser.h"
#include "headers/texture.h"
#include "headers/shadow.h"
#include "headers/proceduralterrain.h"
#include "headers/subVertices.h"
#include "headers/propellerVertices.h"
#include "headers/movable.h"
#include "headers/casteljau.h"
#include "headers/point.h"

using namespace std;

// light 1
glm::vec3 lightDirection1;
glm::vec3 lightPos1;
float lightPitch;
bool lightEnabled = true;

// light 2
glm::vec3 lightPos2;


Movable::movable light;
Movable::movable submarine;

// jellyfish position relied on by its pink glow (positional light) and its model
glm::mat4 jellyFishPos; 

FTCamera::FTCamera Camera;
bool firstPerson = false; // first person camera bool value

#define NUM_BUFFERS 11
#define NUM_VAOS 11
#define NUM_OBJ 11
GLuint Buffers[NUM_BUFFERS];
GLuint VAOs[NUM_VAOS];
int objSizes[NUM_OBJ];


#define WIDTH 1920
#define HEIGHT 1080

#define SH_MAP_WIDTH 2048
#define SH_MAP_HEIGHT 2048

#define TERRAIN_X 1000
#define TERRAIN_Z 1000

#define TERRAIN_X_STEP 2.5f
#define TERRAIN_Z_STEP 2.5f

double lastMouseX = HEIGHT / 2;
double lastMouseY = WIDTH / 2;

bool mouseFirstTime = true;

void cameraMouseCallback(GLFWwindow* window, double mouseX, double mouseY) {
	if (!firstPerson) {
		if (mouseFirstTime) {
			lastMouseX = mouseX;
			lastMouseY = mouseY;
			mouseFirstTime = false;
		}
		float yawOffset = mouseX - lastMouseX;
		float pitchOffset = lastMouseY - mouseY;

		lastMouseX = mouseX;
		lastMouseY = mouseY;
		FTCamera::MoveAndOrientFTCamera(Camera, 0.f, 0.f, yawOffset, pitchOffset);
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float frontOffset = 0.f;
	float rightOffset = 0.f;

	float subFrontOffset = 0.f;
	float subRightOffset = 0.f;
	float subUpOffset = 0.f;
	float subYawOffset = 0.f;
	float subPitchOffset = 0.f;

	bool cam_changed = false;

	bool sub_changed = false;

	if (key == GLFW_KEY_D && action == GLFW_REPEAT) {
		rightOffset = 1.f;
		frontOffset = 0.f;
		cam_changed = true;
	}

	if (key == GLFW_KEY_A && action == GLFW_REPEAT) {
		rightOffset = -1.f;
		frontOffset = 0.f;
		cam_changed = true;
	}

	if (key == GLFW_KEY_W && action == GLFW_REPEAT) {
		rightOffset = 0.f;
		frontOffset = 1.f;
		cam_changed = true;
	}

	if (key == GLFW_KEY_S && action == GLFW_REPEAT) {
		rightOffset = 0.f;
		frontOffset = -1.f;
		cam_changed = true;
	}

	if (key == GLFW_KEY_UP && action == GLFW_REPEAT) {
		subRightOffset = 0.f;
		subFrontOffset = 1.0f;
		sub_changed = true;
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_REPEAT) {
		subYawOffset = 1.f;
		sub_changed = true;
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_REPEAT) {
		subYawOffset = -1.f;
		sub_changed = true;
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_REPEAT) {
		subRightOffset = 0.f;
		subFrontOffset = -1.f;
		sub_changed = true;
	}

	if (key == GLFW_KEY_M && action == GLFW_REPEAT) {
		subUpOffset = -1.f;
		sub_changed = true;
	}

	if (key == GLFW_KEY_K && action == GLFW_REPEAT) {
		subUpOffset = 1.f;
		sub_changed = true;
	}


	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		if (firstPerson) {
			firstPerson = false;
		}
		else {
			Camera.Position = submarine.position;
			Camera.Front = submarine.Front;
			Camera.Position += submarine.Front * 60.f;
			firstPerson = true;
		}
	}

	if (key == GLFW_KEY_I && action == GLFW_REPEAT) {
		if (lightPitch < 45.f) {
			lightPitch += 1.f;
			lightDirection1.y = glm::sin(glm::radians(lightPitch));
		}
	}
	if (key == GLFW_KEY_J && action == GLFW_REPEAT) {
		if (lightPitch > -45.f) {
			lightPitch -= 1.f;
			lightDirection1.y = glm::sin(glm::radians(lightPitch));
		}
	}

	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		if (lightEnabled) {
			lightDirection1 = glm::vec3(0.f, 1.f, 0.f);
			lightPos1 = glm::vec3(999.f, 999.f, 999.f);
			lightEnabled = false;
		}
		else {
			lightDirection1 = submarine.Front;
			lightPos1 = submarine.position;
			lightPos1 += submarine.Front * 40.f;
			lightPos1 += submarine.Up * 50.f;
			lightDirection1.y = glm::sin(glm::radians(lightPitch));
			lightEnabled = true;
		}
	}

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		lightDirection1 = Camera.Front;
		lightPos1 = Camera.Position;
	}

	if (cam_changed) {
		if (!firstPerson)
			FTCamera::MoveAndOrientFTCamera(Camera, frontOffset, rightOffset, 0.f, 0.f);
	}

	if (sub_changed) {
		Movable::moveAndOrient(submarine, subFrontOffset, subUpOffset, subRightOffset, subYawOffset, 0.f);
		if (lightEnabled) {
			lightDirection1 = submarine.Front;
			lightPos1 = submarine.position;
			lightPos1 += submarine.Front * 40.f;
			lightPos1 += submarine.Up * 50.f;
			lightDirection1.y = glm::sin(glm::radians(lightPitch));
		}
		if (firstPerson) {
			Camera.Position = submarine.position;
			Camera.Front = submarine.Front;
			Camera.Position += submarine.Front * 60.f;
		}
	}
}

void SizeCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

void drawSingleTexturedModel(glm::mat4 modelMatrix, vector<GLuint> textures, int VAOIdx, int textureIdx, int objSizeIdx, unsigned int shaderProgram) {
	glBindVertexArray(VAOs[VAOIdx]);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, textures[textureIdx]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glDrawArrays(GL_TRIANGLES, 0, objSizes[objSizeIdx]);
}

void drawTexturedPhongModels(unsigned int shaderProgram, vector<GLuint> textures) {
	glUniform1f(glGetUniformLocation(shaderProgram, "ambient"), 50.f);
	glUniform1f(glGetUniformLocation(shaderProgram, "brightness"), 0.3f);
	glUniform1f(glGetUniformLocation(shaderProgram, "diffuseMultiplier1"), 5000.f); //spotlight diffuse
	glUniform1f(glGetUniformLocation(shaderProgram, "diffuseMultiplier2"), 300.f); //jellyfish glow diffuse

	glm::mat4 model = glm::mat4(1.f);

	// fish
	float fishAngleSpeed = 0.3f;
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(0.f, sin((float)glfwGetTime()/2.f) * 50.f, 200.f));
	model = glm::translate(model, glm::vec3(-150.f * sin((float)glfwGetTime() * fishAngleSpeed), 0.f, -150.f * cos((float)glfwGetTime() * fishAngleSpeed)));
	model = glm::rotate(model, (float)glfwGetTime() * fishAngleSpeed, glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	drawSingleTexturedModel(model, textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(-10.f, 50.f, 0.f)), textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(-30.f,-50.f, 0.f)), textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(-30.f, 0.f, 20.f)), textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(50.f, 0.f, -40.f)), textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(-25.f, 0.f, -40.f)), textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(-25.f, 30.f, -40.f)), textures, 0, 0, 0, shaderProgram);
	drawSingleTexturedModel(glm::translate(model, glm::vec3(30.f, 30.f, 30.f)), textures, 0, 0, 0, shaderProgram);

	// whale 
	float whaleAngleSpeed = 0.1f;
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-600.f * sin((float)glfwGetTime() * whaleAngleSpeed), 350.f, -600.f * cos((float)glfwGetTime() * whaleAngleSpeed)));
	model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
	model = glm::rotate(model, (float)glfwGetTime() * whaleAngleSpeed, glm::vec3(0.f, 0.f, 1.f));
	drawSingleTexturedModel(model, textures, 1, 1, 1, shaderProgram);


	//terrain
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-(TERRAIN_X * TERRAIN_X_STEP) / 2, -300.f, -(TERRAIN_Z * TERRAIN_Z_STEP) / 2));
	drawSingleTexturedModel(model, textures, 2, 2, 2, shaderProgram);

	//submarine
	model = glm::mat4(1.f);
	model = glm::translate(model, submarine.position);
	model = glm::scale(model, glm::vec3(10.f,10.f,10.f));
	model = glm::rotate(model, glm::radians(submarine.Yaw), glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, glm::radians(-90.f), glm::vec3(0.f, 1.f, 0.f));
	drawSingleTexturedModel(model, textures, 3, 3, 3, shaderProgram);

	//propeller
	model = glm::mat4(1.f);
	model = glm::translate(model, submarine.position - submarine.Front * 85.f);
	model = glm::rotate(model, glm::radians(submarine.Yaw), submarine.Up);
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.0f, 0.f, 0.f));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
	model = glm::scale(model, glm::vec3(5.f, 5.f, 5.f));
	model = glm::rotate(model, (float)glfwGetTime()* 2, glm::vec3(0.f, 1.f, 0.f));
	drawSingleTexturedModel(model, textures, 4, 4, 4, shaderProgram);

	////boat
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-250.f, -250.f, -250.f));
	model = glm::rotate(model, glm::radians(35.f), glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, glm::radians(35.f), glm::vec3(1.f, 0.f, 0.f));
	drawSingleTexturedModel(model, textures, 5, 5, 5, shaderProgram);

	//columns
	glm::mat4 column1, column2;
	model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(350.f, -300.f, 250.f));

	column1 = glm::rotate(model, glm::radians(-50.f), glm::vec3(0.f, 0.f, 1.f));
	column1 = glm::scale(column1, glm::vec3(50.f, 50.f, 50.f));
	drawSingleTexturedModel(column1, textures, 6, 6, 6, shaderProgram);

	column2 = glm::translate(model, glm::vec3(-90.f, -100.f, 150.f));
	column2 = glm::scale(column2, glm::vec3(50.f, 50.f, 50.f));
	column2 = glm::rotate(column2, glm::radians(-20.f), glm::vec3(0.f, 1.f, 1.f));
	drawSingleTexturedModel(column2, textures, 6, 6, 6, shaderProgram);
	
	// jelly fish head
	model = glm::translate(jellyFishPos, glm::vec3(0.f, 98.f, 0.f));
	model = glm::scale(model, glm::vec3(8.f, 8.f, 8.f));
	drawSingleTexturedModel(model, textures, 7, 7, 7, shaderProgram);
}

void drawPhongModels(unsigned int shaderProgram, vector<GLuint> textures) {
	glm::mat4 model = glm::mat4(1.f);
	model = glm::translate(model, glm::vec3(-(TERRAIN_X * TERRAIN_X_STEP)/2, -300.f, -(TERRAIN_Z * TERRAIN_Z_STEP) / 2));
	glBindVertexArray(VAOs[2]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_TRIANGLES, 0, objSizes[2]);
}

void setUpBuffer(GLuint bufferElem, GLuint VAOElem, int objSize, float* vertices) {
	glNamedBufferStorage(bufferElem, sizeof(float)* objSize * 9, vertices, 0);
	glBindVertexArray(VAOElem);
	glBindBuffer(GL_ARRAY_BUFFER, bufferElem);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (9 * sizeof(float)), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void setUpCurvesBuffer(GLuint bufferElem, GLuint VAOElem, int num_curve_floats, float* curve_verts) {
	glBindVertexArray(VAOElem);
	glNamedBufferStorage(bufferElem, sizeof(float) * num_curve_floats, curve_verts, 0);
	glBindBuffer(GL_ARRAY_BUFFER, bufferElem);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 * sizeof(float)), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}


int main(int argc, char** argv)
{
	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 8);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Underwater", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetWindowSizeCallback(window, SizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cameraMouseCallback);

	gl3wInit();

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(DebugCallback, 0);

	// setup shadow map and load shadow shaders
	ShadowStruct shadow = setup_shadowmap(SH_MAP_WIDTH, SH_MAP_HEIGHT);
	GLuint shadowProgram = CompileShader("shaders/shadow.vert", "shaders/shadow.frag");

	// load shader programs
	GLuint plainProgram = CompileShader("shaders/triangle.vert", "shaders/triangle.frag");
	GLuint texturedPhongProgram = CompileShader("shaders/texturedphong.vert", "shaders/texturedphong.frag");

	Movable::init(submarine, glm::vec3(300.f, 0.f, 0.f), 180.f, 0.f, 2.f, 5.f, 1.f);
	InitCamera(Camera);

	lightDirection1 = submarine.Front;
	lightPos1 = submarine.position;
	lightPos1 += submarine.Front * 40.f;
	lightPos1 += submarine.Up * 50.f;
	lightPitch = -45.f;
	lightDirection1.y = glm::sin(glm::radians(lightPitch));

	// load textures
	vector<GLuint> textures;
	GLuint fishTexture = setup_texture("imageAssets/fish.bmp");
	GLuint whaleTexture = setup_texture("imageAssets/whale.bmp");
	GLuint sandTexture = setup_texture("imageAssets/seamless_beach_sand_texture.bmp");
	GLuint subTexture = setup_texture("imageAssets/subtexture4.bmp");
	GLuint propellerTexture = setup_texture("imageAssets/propeller.bmp");
	GLuint boatTexture = setup_texture("imageAssets/boat_diffuse.bmp");
	GLuint columnTexture = setup_texture("imageAssets/map_Column00001_BaseColor.bmp");
	GLuint jellyfishTexture = setup_texture("imageAssets/jellyfishhead.bmp");
	textures.push_back(fishTexture);
	textures.push_back(whaleTexture);
	textures.push_back(sandTexture);
	textures.push_back(subTexture);
	textures.push_back(propellerTexture);
	textures.push_back(boatTexture);
	textures.push_back(columnTexture);
	textures.push_back(jellyfishTexture);

	//fish
	float* fishObj = parseObjFile(objSizes[0], "objFiles/fish.obj", true);
	objSizes[0] /= 9;

	//whale
	float* whaleObj = parseObjFile(objSizes[1], "objFiles/whale.obj", true);
	objSizes[1] /= 9;

	// procedural terrain
	float* terrain = generateMeshVerts(objSizes[2], TERRAIN_X, TERRAIN_Z, TERRAIN_X_STEP, TERRAIN_Z_STEP, 20000.f, 20000.f);
	objSizes[2] /= 9;

	//submarine size
	objSizes[3] = (74646 * 3) / 9;

	//propeller size
	objSizes[4] = (15948 * 3) / 9;

	//boat
	float* boat = parseObjFile(objSizes[5], "objFiles/Boat.obj", false);
	objSizes[5] /= 9;

	//column
	float* column = parseObjFile(objSizes[6], "objFiles/columns_001.obj", false);
	objSizes[6] /= 9;

	// jelly fish head
	float* jellyfishhead= parseObjFile(objSizes[7], "objFiles/jellyfishhead.obj", false);
	objSizes[7] /= 9;

	glCreateBuffers(NUM_BUFFERS, Buffers);
	glGenVertexArrays(NUM_VAOS, VAOs);

	setUpBuffer(Buffers[0], VAOs[0], objSizes[0], fishObj);
	setUpBuffer(Buffers[1], VAOs[1], objSizes[1], whaleObj);
	setUpBuffer(Buffers[2], VAOs[2], objSizes[2], terrain);
	setUpBuffer(Buffers[3], VAOs[3], objSizes[3], subVerts);
	setUpBuffer(Buffers[4], VAOs[4], objSizes[4], propellerVertices);
	setUpBuffer(Buffers[5], VAOs[5], objSizes[5], boat);
	setUpBuffer(Buffers[6], VAOs[6], objSizes[6], column);
	setUpBuffer(Buffers[7], VAOs[7], objSizes[7], jellyfishhead);

	//setup jelly fish tentacles as curves
	float coilRadius = 50.f;
	int num_evaluations = 30;

	std::vector<point> ctrl_points1;
	ctrl_points1.push_back(point(0.f, 100.f, 0.f));
	ctrl_points1.push_back(point(15.f, 90.f, 0.f));
	ctrl_points1.push_back(point(-10.f, 75.f, 0.f));
	ctrl_points1.push_back(point(-10.f, 25.f, 0.f));
	ctrl_points1.push_back(point(20.f, 0.f, 0.f));

	std::vector<point> ctrl_points2;
	ctrl_points2.push_back(point(0.f, 100.f, 0.f));
	ctrl_points2.push_back(point(coilRadius/2, 87.f, coilRadius/2));
	ctrl_points2.push_back(point(-coilRadius, 75.f, coilRadius));
	ctrl_points2.push_back(point(-coilRadius, 63.f, -coilRadius));
	ctrl_points2.push_back(point(coilRadius, 50.f, -coilRadius));
	ctrl_points2.push_back(point(coilRadius, 38.f, coilRadius));
	ctrl_points2.push_back(point(-coilRadius, 25.f, coilRadius));
	ctrl_points2.push_back(point(-coilRadius/2, 12.f, -coilRadius/2));
	ctrl_points2.push_back(point(0.f, 0.f, 0.f));

	std::vector<point> curve1 = EvaluateBezierCurve(ctrl_points1, num_evaluations);
	std::vector<point> curve2 = EvaluateBezierCurve(ctrl_points2, num_evaluations);

	int num_curve_verts1 = 0;
	int num_curve_floats1 = 0;
	int num_curve_verts2 = 0;
	int num_curve_floats2 = 0;
	float* curve_vertices1 = MakeFloatsFromVector(curve1, num_curve_verts1, num_curve_floats1, 0.5f, 0.17f, 0.5f);
	float* curve_vertices2 = MakeFloatsFromVector(curve2, num_curve_verts2, num_curve_floats2, 0.4f, 0.f, 0.4f);

	setUpCurvesBuffer(Buffers[9], VAOs[9], num_curve_floats1, curve_vertices1);
	setUpCurvesBuffer(Buffers[10], VAOs[10], num_curve_floats2, curve_vertices2);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		static const GLfloat bgd[] = { 0.f, 0.f, 0.2f, 1.f };
		glClearBufferfv(GL_COLOR, 0, bgd);


		float near_plane = 0.0f, far_plane = 2000.f;
		glm::mat4 lightProjection = glm::ortho(-300.f, 300.f, -300.f, 300.f, near_plane, far_plane);
		glm::mat4 lightView = glm::lookAt(lightPos1, lightPos1 + lightDirection1, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 projectedLightSpaceMatrix = lightProjection * lightView;

		generateDepthMap(shadowProgram, shadow, projectedLightSpaceMatrix, textures, SH_MAP_WIDTH, SH_MAP_HEIGHT, &drawTexturedPhongModels);
		renderWithShadow(Camera, lightPos1, lightDirection1, lightPos2, texturedPhongProgram, shadow, projectedLightSpaceMatrix, textures, WIDTH, HEIGHT, &drawTexturedPhongModels, true);

		// jellfish tentacles as curves
		jellyFishPos = glm::mat4(1.f);
		jellyFishPos = glm::translate(jellyFishPos, glm::vec3(400.f, -100.f, -100.f));
		jellyFishPos = glm::translate(jellyFishPos, glm::vec3(0.f, sin((float)glfwGetTime()/2) * 100.f, 0.f));
		lightPos2 = glm::vec3(400.f, 20.f, -100.f) + glm::vec3(0.f, sin((float)glfwGetTime()/2) * 100.f, 0.f);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUseProgram(plainProgram);

		glm::mat4 view = glm::mat4(1.f);
		view = glm::lookAt(Camera.Position, Camera.Position + Camera.Front, Camera.Up);
		glUniformMatrix4fv(glGetUniformLocation(plainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 projection = glm::mat4(1.f);
		projection = glm::perspective(glm::radians(45.f), (float)WIDTH / (float)HEIGHT, .01f, 10000.f);
		glUniformMatrix4fv(glGetUniformLocation(plainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		float distanceCamJellyfish = glm::distance(Camera.Position, glm::vec3(400.f, -100.f, -100.f) + glm::vec3(0.f, sin((float)glfwGetTime() / 2) * 100.f, 0.f));

		for (int tendrilIdx = 0; tendrilIdx < 10; tendrilIdx++) {
			glm::mat4 tendrilRelativePos = glm::translate(jellyFishPos, glm::vec3(cos(glm::radians(36.f * tendrilIdx)) * 25.f, 0.f, sin(glm::radians(36.f * tendrilIdx)) * 25.f));
			tendrilRelativePos = glm::rotate(tendrilRelativePos, glm::radians(-36.f * tendrilIdx), glm::vec3(0.f, 1.f, 0.f));
			glUniformMatrix4fv(glGetUniformLocation(plainProgram, "model"), 1, GL_FALSE, glm::value_ptr(tendrilRelativePos));
			glBindVertexArray(VAOs[9]);
			glLineWidth(5000.f / max(distanceCamJellyfish, 0.01f));
			glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts1);
		}

		glUniformMatrix4fv(glGetUniformLocation(plainProgram, "model"), 1, GL_FALSE, glm::value_ptr(jellyFishPos));
		glBindVertexArray(VAOs[10]);
		glLineWidth(5000.f/max(distanceCamJellyfish, 0.01f));
		glDrawArrays(GL_LINE_STRIP, 0, num_curve_verts2);

		//ending loop
		glBindVertexArray(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}