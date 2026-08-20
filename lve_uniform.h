#pragma once
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include<vulkan/vulkan.h>
#include<iostream>
#include"lve_device.h"
namespace lve {
	class LveUniform
	{
	public:
		void createDescriptorSetLayout(VkDevice device);//创建描述符集布局
		void createUniformBuffer(uint32_t MAX_FRAMES_IN_FLIGHT, LveDevice& device);//创建ubo
		void createDescriptorPool(uint32_t MAX_FRAMES_IN_FLIGHT, VkDevice device);//创建描述符池
		void createDescriptorSets(uint32_t MAX_FRAMES_IN_FLIGHT, VkDevice device);//创建描述符集
		void clean(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT);
		void updateUniformBuffer(uint32_t currentImage, VkExtent2D extent, const glm::mat4& modelMatrix, const glm::mat4& view, const glm::mat4& proj);
		VkDescriptorSetLayout getDescriptorSetLayout() { return descriptorSetLayout; };
		const std::vector<VkDescriptorSet>&
			getDescriptorSets() const noexcept {
			return descriptorSets;
		}
	private:
		struct UniformBufferObject {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};
		VkDescriptorSetLayout descriptorSetLayout;//描述符布局
		VkDescriptorPool descriptorPool;//描述符池
		std::vector<VkDescriptorSet> descriptorSets;//描述符
		std::vector<VkDescriptorSetLayout> layouts;//对应布局

		std::vector<VkBuffer> uniformBuffers;//uniform的缓冲区
		std::vector<VkDeviceMemory> uniformBuffersMemory;//uniform的缓冲区内存
		std::vector<void*> uniformBuffersMapped;//uniformBuffer对应的cpu地址

	};
}