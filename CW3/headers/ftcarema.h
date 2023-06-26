#pragma once

#include <stdio.h>
#include <glm/glm/glm.hpp>
#include <cmath>

#define PI 3.14f

#define DEG2RAD(n) n*(PI/180)
#define RAD2DEG(n) n*(180/PI)

namespace FTCamera {
	struct FTCamera
	{
		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;

		glm::vec3 WorldUp;

		float Yaw;
		float Pitch;

		const float MovementSpeed = 10.f;
		float MouseSensitivity = 0.1f;
	};

	void InitCamera(FTCamera& cam)
	{
		cam.Front = glm::vec3(0.0f, 0.0f, -1.0f);
		cam.Position = glm::vec3(0.0f, 0.0f, 750.f); //500 at z
		cam.Up = glm::vec3(0.0f, 1.0f, 0.0f);
		cam.WorldUp = cam.Up;
		cam.Right = glm::normalize(glm::cross(cam.Front, cam.WorldUp));

		cam.Yaw = -90.f;
		cam.Pitch = 0.f;
	}

	void MoveAndOrientFTCamera(FTCamera& cam, float frontOffset, float rightOffset, float yawOffset, float pitchOffset) {

		yawOffset *= cam.MouseSensitivity;
		pitchOffset *= cam.MouseSensitivity;

		cam.Yaw += yawOffset;
		cam.Pitch += pitchOffset;

		if (cam.Pitch > 89.0f) cam.Pitch = 89.0f;
		if (cam.Pitch < -89.0f) cam.Pitch = -89.0f;

		cam.Front.x = cos(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
		cam.Front.y = sin(glm::radians(cam.Pitch));
		cam.Front.z = sin(glm::radians(cam.Yaw)) * cos(glm::radians(cam.Pitch));
		cam.Front = glm::normalize(cam.Front);

		cam.Position += cam.MovementSpeed * frontOffset * cam.Front;

		cam.Right = glm::normalize(glm::cross(cam.Front, cam.WorldUp));

		cam.Position += cam.MovementSpeed * rightOffset * cam.Right;
	}
	
}