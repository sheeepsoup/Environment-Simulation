#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE//定义glm的深度缓冲是从0~1而不是gl的-1~1
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include<iostream>
namespace lve {
	class LveCamera
	{
	public:

		void setViewDirection(glm::vec3 newposition, glm::vec3 newdirection, glm::vec3 newup);
		void updateViewMatrix();
		void setPerspectiveProjection(float fovY, float aspect, float nearPlane, float farPlane);
		const glm::mat4& getView() const {return viewMatrix;}
		void forward_and_behind(bool forwards, float deltaTime);//处理向前或者后 true->前 false->后
		void right_and_left(bool rights, float deltaTime);//向右或者向左 true->右 false->左
		void up_and_down(bool up, float deltaTime);//向上或者向下 true->上 false->下
		void rotate(float mouseDeltaX, float mouseDeltaY);//旋转摄像机
		const glm::mat4& getProjection() const {return projectionMatrix;}
		glm::vec3 getPos() { return postion; };
	private:
		float speed = 16.0f;
		glm::vec3 postion{0.0f};
		glm::vec3 direction;
		glm::vec3 worldUp{ 0.0f, 0.0f, 1.0f };
		glm::mat4 viewMatrix{1.0f};
		glm::mat4 projectionMatrix{1.0f};
		float yaw = 0.0f;//水平旋转角
		float pitch = 0.0f;//垂直旋转角
		float sensitivity = 0.1f;//鼠标灵敏度

	};



}