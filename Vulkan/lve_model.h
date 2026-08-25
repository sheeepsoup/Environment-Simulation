#pragma once
#include<vulkan/vulkan.h>
#include<glm/glm.hpp>
#include <stdint.h>
#include<iostream>
#include <array>
#include<vector>
#include<FastNoiseLite.h>
#include <random>
#include"lve_device.h"

namespace lve {
	class LveModel {
	public:
		struct Vertex {
			glm::vec3 pos;
			glm::vec3 color;
			glm::vec3 normal;
			float flow;
			static VkVertexInputBindingDescription getBindingDescription() {//一个完整的顶点在内存里占多少字节，以及绘制时应该按“每个顶点”还是“每个实例”读取下一条数据
				VkVertexInputBindingDescription bindingDescription{};
				bindingDescription.binding = 0;//使用槽位0
				bindingDescription.stride = sizeof(Vertex);//每个顶点占用的字节数
				bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;//每个顶点读取一次
				return bindingDescription;
			}
			static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {//有3个属性:位置和颜色
				std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
				// position
				attributeDescriptions[0].binding = 0;//每个顶点的数据来自哪个绑定
				attributeDescriptions[0].location = 0;//顶点着色器中位置为0
				attributeDescriptions[0].format =
					VK_FORMAT_R32G32B32_SFLOAT;//这个格式看vulkan文档,float是R,vec2 RG vec3 RGB vec4 RGBA[float类是R32 double类是R64]
				attributeDescriptions[0].offset =
					offsetof(Vertex, pos);

				// color
				attributeDescriptions[1].binding = 0;//每个顶点的数据来自哪个绑定
				attributeDescriptions[1].location = 1;//顶点着色器中位置为1
				attributeDescriptions[1].format =
					VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[1].offset =
					offsetof(Vertex, color);

				//normal
				attributeDescriptions[2].binding = 0;
				attributeDescriptions[2].location = 2;
				attributeDescriptions[2].format =
					VK_FORMAT_R32G32B32_SFLOAT;
				attributeDescriptions[2].offset =
					offsetof(Vertex, normal);

				//流量
				attributeDescriptions[3].binding = 0;
				attributeDescriptions[3].location = 3;
				attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
				attributeDescriptions[3].offset = offsetof(Vertex, flow);

				return attributeDescriptions;
			}

		};


		void clean(VkDevice device);
		void createVertexBuffer(LveDevice &device, std::vector<Vertex>& vertices);//这是持续cpu上传的函数			[二选一] 用途:用于顶点在cpu端频繁更新的情况,例如粒子系统,动态顶点等
		void createVertexBufferWithStaging(LveDevice &lveDevice, std::vector<Vertex>& vertices);//这是上传到gpu内存后用的函数[二选一] 用途:用于顶点在cpu端不频繁更新的情况,例如静态模型,静态网格等
		void createIndexBufferWithStaging(LveDevice& lveDevice, std::vector<uint32_t>& indices);//创建索引缓冲区
		void bindVertex(VkCommandBuffer commandBuffer);//绑定vertex
		void bindIndexBuffer(VkCommandBuffer commandBuffer);//绑定索引缓冲区
	
	

	private:
		VkBuffer vertexBuffer = VK_NULL_HANDLE;//顶点缓冲区
		VkBuffer stagingBuffer = VK_NULL_HANDLE;//暂存缓冲区
		VkBuffer indexBuffer = VK_NULL_HANDLE;//索引缓冲区
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;//索引缓冲区内存
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;//顶点缓冲区内存[cpu上]
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;//暂存缓冲区内存[gpu上]
		
		void* data;//指向顶点数据的指针[就是下面的vertices]



		
		void fillInVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkDeviceMemory bufferMemory, std::vector<Vertex>& vertices);//填充顶点缓冲区
	
		
		struct TerrainDeltaGrid //线程专用的土地属性
		{
			std::vector<float> detailHeight;//对应顶点高度
			inline void addDelta(size_t index, float amount) {
				detailHeight[index] += amount;
			}
			inline float getDelta(size_t index) const {
				return detailHeight[index];
			}
			void reset() {
				std::fill(detailHeight.begin(), detailHeight.end(), 0.0f);
			}
		};


	};
}