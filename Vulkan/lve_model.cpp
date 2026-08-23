#include"lve_model.h"

namespace lve {
	void LveModel::createVertexBuffer(LveDevice device, std::vector<Vertex> &vertices) {
		//创建缓冲区
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		device.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					vertexBuffer, vertexBufferMemory);

	
		fillInVertexBuffer(device.getDevice(), device.getPhysicalDevice(), bufferSize, vertexBufferMemory,vertices);//填充顶点缓冲区

	}

	void LveModel::createVertexBufferWithStaging(LveDevice &lveDevice, const std::vector<Vertex>& vertices) {
		//创建一个临时缓冲区,用于将数据从CPU传输到GPU
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
		//创建临时缓冲区
		lveDevice.createBuffer(bufferSize
			, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory
		);

		fillInVertexBuffer(lveDevice.getDevice(), lveDevice.getPhysicalDevice(), bufferSize, stagingBufferMemory,vertices);//填充缓冲区

		//创建顶点缓冲区
		lveDevice.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory
		);

		lveDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		vkDestroyBuffer(
			lveDevice.getDevice(),
			stagingBuffer,
			nullptr
		);

		vkFreeMemory(
			lveDevice.getDevice(),
			stagingBufferMemory,
			nullptr
		);

		stagingBuffer = VK_NULL_HANDLE;
		stagingBufferMemory = VK_NULL_HANDLE;
	}
	void LveModel::fillInVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize,VkDeviceMemory bufferMemory
		, const std::vector<Vertex>& vertices) {
		vkMapMemory(device, bufferMemory, 0, bufferSize, 0, &data);//映射内存
		memcpy(data, vertices.data(), (size_t)bufferSize);//拷贝数据到映射内存
		vkUnmapMemory(device, bufferMemory);//解除映射内存
	};
	void LveModel::bindVertex(VkCommandBuffer commandBuffer) {
		//绑定顶点缓冲区
		VkBuffer vertexBuffers[] = { vertexBuffer };//顶点缓冲区数组
		VkDeviceSize offsets[] = { 0 };//偏移量数组
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);//绑定顶点缓冲区
	}
	void LveModel::bindIndexBuffer(VkCommandBuffer commandBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}
	void LveModel::clean(VkDevice device) {
		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);
	}

	void LveModel::createIndexBufferWithStaging(LveDevice& lveDevice, const std::vector<uint32_t> &indices) {
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(lveDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(lveDevice.getDevice(), stagingBufferMemory);

		lveDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		lveDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(lveDevice.getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(lveDevice.getDevice(), stagingBufferMemory, nullptr);
	}
	
	void LveModel::draw_index_mode(VkCommandBuffer commandBuffer, std::vector<uint32_t>& indices) {
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}
	void LveModel::draw(VkCommandBuffer commandBuffer, std::vector<Vertex>& vertices) {
		vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
	}

	void LveModel::destroyMeshBuffers(
		VkDevice device
	) {
		if (vertexBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(
				device,
				vertexBuffer,
				nullptr
			);

			vertexBuffer = VK_NULL_HANDLE;
		}

		if (vertexBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(
				device,
				vertexBufferMemory,
				nullptr
			);

			vertexBufferMemory = VK_NULL_HANDLE;
		}

		if (indexBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(
				device,
				indexBuffer,
				nullptr
			);

			indexBuffer = VK_NULL_HANDLE;
		}

		if (indexBufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(
				device,
				indexBufferMemory,
				nullptr
			);

			indexBufferMemory = VK_NULL_HANDLE;
		}
	}
	void LveModel::replaceMesh(
		LveDevice& device,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices
	) {
		destroyMeshBuffers(
			device.getDevice()
		);

		createVertexBufferWithStaging(
			device,
			vertices
		);

		createIndexBufferWithStaging(
			device,
			indices
		);
	}
}