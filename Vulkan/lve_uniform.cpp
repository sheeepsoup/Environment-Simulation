#include"lve_uniform.h"

namespace lve {
	void LveUniform::createDescriptorSetLayout(VkDevice device) {
		VkDescriptorSetLayoutBinding uboLayoutBinding{};//创建结构体
		uboLayoutBinding.binding = 0;//占用描述符集合中的binding0[就是shader里的binding那个]
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;//绑定的是VkBuffer
		uboLayoutBinding.descriptorCount = 1;//有一个描述符
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;//在顶点着色器阶段使用
		uboLayoutBinding.pImmutableSamplers = nullptr; //图像采样描述符,暂时默认

		VkDescriptorSetLayoutCreateInfo layoutInfo{};//信息
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {//创建
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}
	void LveUniform::clean(VkDevice device, uint32_t MAX_FRAMES_IN_FLIGHT) {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}
	void LveUniform::createUniformBuffer(uint32_t MAX_FRAMES_IN_FLIGHT,LveDevice& device) {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		//为每一帧创建独立的ubo
		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {//为每个飞行的帧加个ubo
			device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

			vkMapMemory(device.getDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);//标记位置
		}

	}
	void LveUniform::updateUniformBuffer(uint32_t currentImage,VkExtent2D extent, const glm::mat4& modelMatrix, const glm::mat4& view, const glm::mat4& proj,glm::vec3 cameraPos) {
	
		//UBO
		UniformBufferObject ubo{};
		ubo.model = modelMatrix;
		ubo.view = view;
		ubo.proj = proj;
		ubo.cameraPos = glm::vec4(cameraPos,1.0f);
		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}
	void LveUniform::createDescriptorPool(uint32_t MAX_FRAMES_IN_FLIGHT,VkDevice device) {
		VkDescriptorPoolSize poolSize{};//大小
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);//数量

		VkDescriptorPoolCreateInfo poolInfo{};//信息
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);//最大数量
		poolInfo.flags = 0;//保持默认
		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {//创建
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
	void LveUniform::createDescriptorSets(uint32_t MAX_FRAMES_IN_FLIGHT,VkDevice device) {
		layouts.resize(MAX_FRAMES_IN_FLIGHT,descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);//描述符集
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {//创建
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {//填充描述符池
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional
			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	}
}