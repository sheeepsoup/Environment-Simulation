#include"lve_camera.h"

namespace lve {
	void LveCamera::setViewDirection(glm::vec3 newposition,glm::vec3 newdirection,glm::vec3 newup) {
		viewMatrix = glm::lookAt (
			newposition,
			newposition + newdirection,
			newup);
		postion = newposition;
		direction = glm::normalize(newdirection);
		worldUp = glm::normalize(newup);

		updateViewMatrix();
	};
	void LveCamera::setPerspectiveProjection(float fovY,float aspect,float nearPlane,float farPlane) {

		projectionMatrix =glm::perspective(fovY, aspect, nearPlane, farPlane);
		//由于vk和gl是反向的
		projectionMatrix[1][1] *= -1.0f;
	}
	void LveCamera::forward_and_behind(bool forwards, float deltaTime) {
		const float distance = speed * deltaTime;
		const glm::vec3 forwardDirection = glm::normalize(direction);

		if (forwards) {
			postion += forwardDirection * distance;
		}
		else {
			postion -= forwardDirection * distance;
		}

		updateViewMatrix();
	}
	void LveCamera::right_and_left(bool rights, float deltaTime) {
		const float distance = speed * deltaTime;

		const glm::vec3 rightDirection =glm::normalize(glm::cross(direction, worldUp));

		if (rights) {
			postion += rightDirection * distance;
		}
		else {
			postion -= rightDirection * distance;
		}

		updateViewMatrix();
	}
	void LveCamera::up_and_down(bool up, float deltaTime) {
		const float distance = speed * deltaTime;

		if (up) {
			postion += worldUp * distance;
		}
		else {
			postion -= worldUp * distance;
		}

		updateViewMatrix();
	}
	void LveCamera::updateViewMatrix() {
		viewMatrix = glm::lookAt(postion,postion + direction,worldUp);
	}
	void LveCamera::rotate(float mouseDeltaX, float mouseDeltaY) {
		yaw -= mouseDeltaX * sensitivity;
		pitch -= mouseDeltaY * sensitivity;
		pitch = glm::clamp(pitch, -89.0f, 89.0f);
		const float yawRadians = glm::radians(yaw);
		const float pitchRadians = glm::radians(pitch);
		direction.x =glm::cos(pitchRadians) * glm::cos(yawRadians);
		direction.y =glm::cos(pitchRadians) * glm::sin(yawRadians);
		direction.z =glm::sin(pitchRadians);
		direction = glm::normalize(direction);
		updateViewMatrix();

	}
}