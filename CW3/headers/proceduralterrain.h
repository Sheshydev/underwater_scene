#pragma once
#pragma once
#include <vector>
#include <math.h>
#include <algorithm>
#include <vector>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

using namespace std;

glm::vec2 constantVector(int val) {
	int h = val & 3;

	if (h == 0)
		return glm::vec2(1.f, 1.f);
	else if (h == 1)
		return glm::vec2(-1.f, 1.f);
	else if (h == 2)
		return glm::vec2(-1.f, -1.f);
	else
		return glm::vec2(1.f, -1.f);
}

vector<int> genPermutationTable() {
	vector<int> permutation;
	int i;

	for (i = 0; i < 256; i++) {
		permutation.push_back(i);
	}

	random_shuffle(permutation.begin(), permutation.end());

	for (i = 0; i < 256; i++) {
		permutation.push_back(permutation[i]);
	}

	return permutation;
}

vector<int> pTable = genPermutationTable();

float lerp(float t, float a1, float a2) {
	return a1 + t * (a2 - a1);
}

float fade(float t) {
	return ((6 * t - 15) * t + 10) * pow(t, 3);
}

// perlin noise implementation and its auxiliary functions above are from https://rtouti.github.io/graphics/perlin-noise-algorithm
float perlinNoise(float x, float y) {
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;

	float xf = x - floor(x);
	float yf = y - floor(y);

	//vectors to input point
	glm::vec2 topRight = glm::vec2(xf - 1.f, yf - 1.f);
	glm::vec2 topLeft = glm::vec2(xf, yf - 1.f);
	glm::vec2 bottomRight = glm::vec2(xf - 1.f, yf);
	glm::vec2 bottomLeft = glm::vec2(xf, yf);

	int valTopRight = pTable[pTable[X + 1] + Y + 1];
	int valTopLeft = pTable[pTable[X] + Y + 1];
	int valBottomRight = pTable[pTable[X + 1] + Y];
	int valBottomLeft = pTable[pTable[X] + Y];

	float dotTopRight = glm::dot(topRight, constantVector(valTopRight));
	float dotTopLeft = glm::dot(topLeft, constantVector(valTopLeft));
	float dotBottomRight = glm::dot(bottomRight, constantVector(valBottomRight));
	float dotBottomLeft = glm::dot(bottomLeft, constantVector(valBottomLeft));

	float u = fade(xf);
	float v = fade(yf);

	return lerp(u, lerp(v, dotBottomLeft, dotTopLeft), lerp(v, dotBottomRight, dotTopRight));
}

vector<float> genHeightVals(int width, int height) {
	vector<float> heightVals;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float val = perlinNoise(x * 0.01, y * 0.01);

			val += 1.0f;
			val /= 2.0f;

			float c = round(val * 255.f);
			heightVals.push_back(c);
		}
	}

	return heightVals;
}

float getPerlinVal(int x, int y, float magnitude) {
	float perlinVal = perlinNoise(x * 0.01, y * 0.01);
	perlinVal += 1.0f;
	perlinVal /= 2.0f;
	perlinVal *= magnitude;

	return perlinVal;
}

void push_back_xyz(float* vert, int idx, float x, float y, float z) {
	vert[idx] = x;
	vert[idx + 1] = y;
	vert[idx + 2] = z;
}

glm::vec3 getNormal(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3) {
	glm::vec3 d1 = v1 - v2;
	glm::vec3 d2 = v1 - v3;

	glm::vec3 norm = glm::cross(d1, d2);
	return glm::normalize(-norm);
}

float* generateMeshVerts(int& size, int width, int height, float xStep, float yStep, float textureHeight, float textureWidth) {
	float* mesh = (float*)malloc(sizeof(float) * width * height * 9 * 6);
	size = width * height * 9 * 6;

	float texRunningHeight = 0.f;
	float texRunningWidth = 0.f;

	float xBaseVert, yBaseVert;
	glm::vec3 norm;

	for (int y = 0; y < height; y++) {
		texRunningHeight += y * yStep;
		for (int x = 0; x < width; x++) {
			texRunningWidth += x * xStep;

			xBaseVert = x * xStep;
			yBaseVert = y * yStep;
			

			norm = getNormal(
				glm::vec3(xBaseVert, getPerlinVal(x * 2, y * 2, 255.f), yBaseVert),
				glm::vec3(xBaseVert + xStep, getPerlinVal(x * 2 + 1, y * 2, 255.f), yBaseVert),
				glm::vec3(xBaseVert, getPerlinVal(x * 2, y * 2 + 1, 255.f), yBaseVert + yStep)
			);

			// first triangle in quad

			push_back_xyz(mesh, ((y * width) + x) * 9 * 6, xBaseVert, getPerlinVal(x, y, 255.f), yBaseVert);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 3,
				std::fmod(texRunningWidth, textureWidth) / textureWidth, 
				std::fmod(texRunningHeight, textureHeight) / textureHeight, 0.f);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 6, norm.x, norm.y, norm.z);


			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 9, xBaseVert + xStep, getPerlinVal(x + 1, y, 255.f), yBaseVert);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 12, 
				std::fmod(texRunningWidth, textureWidth) / textureWidth,
				std::fmod(texRunningHeight + y * yStep, textureHeight) / textureHeight, 0.f);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 15, norm.x, norm.y, norm.z);

			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 18, xBaseVert, getPerlinVal(x, y + 1, 255.f), yBaseVert + yStep);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 21, 
				std::fmod(texRunningWidth + x * xStep, textureWidth) / textureWidth,
				std::fmod(texRunningHeight, textureHeight) / textureHeight, 0.f);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 24, norm.x, norm.y, norm.z);

			//second triangle in quad
			norm = getNormal(
				glm::vec3(xBaseVert + xStep, getPerlinVal(x * 2 + 1, y * 2, 255.f), yBaseVert),
				glm::vec3(xBaseVert + xStep, getPerlinVal(x * 2 + 1, y * 2 + 1, 255.f), yBaseVert + yStep),
				glm::vec3(xBaseVert, getPerlinVal(x * 2, y * 2 + 1, 255.f), yBaseVert + yStep)
			);

			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 27, xBaseVert + xStep, getPerlinVal(x + 1, y, 255.f), yBaseVert);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 30, 
				std::fmod(texRunningWidth, textureWidth) / textureWidth,
				std::fmod(texRunningHeight + y * yStep, textureHeight) / textureHeight, 0.f);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 33, norm.x, norm.y, norm.z);

			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 36, xBaseVert + xStep, getPerlinVal(x + 1, y + 1, 255.f), yBaseVert + yStep);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 39, 
				std::fmod(texRunningWidth + x * xStep, textureWidth) / textureWidth,
				std::fmod(texRunningHeight + y * yStep, textureHeight) / textureHeight, 0.f);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 42, norm.x, norm.y, norm.z);

			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 45, xBaseVert, getPerlinVal(x, y + 1, 255.f), yBaseVert + yStep);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 48, 
				std::fmod(texRunningWidth + x * xStep, textureWidth) / textureWidth,
				std::fmod(texRunningHeight, textureHeight) / textureHeight, 0.f);
			push_back_xyz(mesh, ((y * width) + x) * 9 * 6 + 51, norm.x, norm.y, norm.z);
		}
		texRunningWidth = 0;
	}

	return mesh;
}
