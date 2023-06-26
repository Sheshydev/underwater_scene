#pragma once
#include <glm/glm/glm.hpp>
#include <cmath>
#include <algorithm>

namespace Movable {
	struct movable {
		glm::vec3 position;
		glm::vec3 Front;
		glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);
		glm::vec3 Right;

		float Yaw;
		float Pitch;

		float moveSpeed;
		float acceleration;
		float deceleration;

		glm::vec3 velocity;
	};

	void init(movable& obj, glm::vec3 pos, float yaw, float pitch, float moveSpeed, float acceleration, float deceleration) {
		obj.position = pos;

		obj.Yaw = yaw;
		obj.Pitch = pitch;
		obj.moveSpeed = moveSpeed;

		obj.Front.x = cos(glm::radians(obj.Yaw)) * cos(glm::radians(obj.Pitch));
		obj.Front.y = sin(glm::radians(obj.Pitch));
		obj.Front.z = sin(glm::radians(obj.Yaw)) * cos(glm::radians(obj.Pitch));
		obj.Front = glm::normalize(obj.Front);
		
		obj.acceleration = acceleration;
		obj.deceleration = deceleration;
	}

	void moveAndOrient(movable& obj, float frontOffset, float upOffset, float rightOffset, float yawOffset, float pitchOffset) {

		obj.Yaw += yawOffset;
		obj.Pitch += pitchOffset;

		obj.Front.x = cos(-glm::radians(obj.Yaw)) * cos(glm::radians(obj.Pitch));
		obj.Front.y = sin(glm::radians(obj.Pitch));
		obj.Front.z = sin(-glm::radians(obj.Yaw)) * cos(glm::radians(obj.Pitch));
		obj.Front = glm::normalize(obj.Front);

		obj.position += obj.moveSpeed * frontOffset * obj.Front;
		obj.Right = glm::normalize(glm::cross(obj.Front, obj.Up));
		obj.position+= obj.moveSpeed * rightOffset * obj.Right;

		obj.position += obj.moveSpeed * upOffset * obj.Up;
	}
}