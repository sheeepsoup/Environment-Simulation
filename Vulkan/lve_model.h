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
		void clean(VkDevice device);
		void createVertexBuffer(LveDevice device);//这是持续cpu上传的函数			[二选一] 用途:用于顶点在cpu端频繁更新的情况,例如粒子系统,动态顶点等
		void createVertexBufferWithStaging(LveDevice lveDevice);//这是上传到gpu内存后用的函数[二选一] 用途:用于顶点在cpu端不频繁更新的情况,例如静态模型,静态网格等
		void createIndexBufferWithStaging(LveDevice& lveDevice);//创建索引缓冲区
		void bindVertex(VkCommandBuffer commandBuffer);//绑定vertex
		void bindIndexBuffer(VkCommandBuffer commandBuffer);//绑定索引缓冲区
		void draw_index_mode(VkCommandBuffer commandBuffer);//索引模式的绘制
		void draw(VkCommandBuffer commandBuffer);//普通绘制
		void initNoise(int seed);//初始化噪声
		void processArea(int seed);//生成地形
		void calculateNormal();//计算法线
		void SetModelSize(uint32_t scale) {//缩放地形大小
			for (auto& vertex : vertices) {
				vertex.pos *= scale;
			}
		}
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
			static std::array<VkVertexInputAttributeDescription,4> getAttributeDescriptions() {//有3个属性:位置和颜色
				std::array<VkVertexInputAttributeDescription,4> attributeDescriptions{};
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
		std::vector<Vertex> vertices;
		std::vector<float> heightData;//高度数据
		int mapVertexCount;//地图顶点大小[x/y方向]
	private:
		VkBuffer vertexBuffer = VK_NULL_HANDLE;//顶点缓冲区
		VkBuffer stagingBuffer = VK_NULL_HANDLE;//暂存缓冲区
		VkBuffer indexBuffer = VK_NULL_HANDLE;//索引缓冲区
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;//索引缓冲区内存
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;//顶点缓冲区内存[cpu上]
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;//暂存缓冲区内存[gpu上]
		
		void* data;//指向顶点数据的指针[就是下面的vertices]
		class CNoise
		{
		public:
			FastNoiseLite biomeNoise;
			FastNoiseLite terrainNoise;
			FastNoiseLite mountainNoise;
			FastNoiseLite detailNoise;
			FastNoiseLite warpNoise;
			FastNoiseLite matrialNoise;//材质噪声,实现草地/岩石等的地域分布稍打乱
			FastNoiseLite snowStream;//雪地的流线图
			FastNoiseLite mountainBaseNoise;
			FastNoiseLite mountainRidgeNoise;
			FastNoiseLite erosionNoise;
		private:

		};
		float smoothstep(float edge0, float edge1, float value);
		CNoise noise;//噪声
		struct WaterDrop
		{
			glm::vec2 position; //当前位置
			glm::vec2 direction;//移动方向
			uint32_t step;      //步数
			float speed;        //速度
			float water;        //剩余水量
			float sediment;     //当前携带的泥沙量
			float maxSediment;  //最大携沙量
		};

		std::vector<WaterDrop> erosion;//腐蚀
		std::vector<uint32_t> indices;//这里indice用于索引缓冲区,数字代表第n个三角形的点,详细问gpt不好解释
		float getHeight(float WorldX, float WorldY, bool isFirst);
		void fillInVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkDeviceMemory bufferMemory);//填充顶点缓冲区
		glm::vec3 WorldUp = glm::vec3(0.0f, 0.0f, 1.0f);
		float terrainHeighLimite = 6.0f;//丘陵的最大高度
		int BlockNum = 50;//区块数量
		int BlockVertexNum = 20;//每个区块x/y对应的顶点数,该区块含有n*n个顶点
		float BlockDistance = 5.0;//每个区块的x/y对应的距离大小
		glm::vec3 calculateNormal(float worldX,float worldY,float sampleDistance);
		glm::vec3 calculateNormalNew(const glm::vec2& gridPosition);//专门用于腐蚀计算的法线计算函数
		glm::vec4 getGroundWeight(float height, glm::vec3 normal, glm::vec2 WorldPos);//获取当前土地的属性权重[r]草 [g]泥土 [b]岩石 [a]雪
		glm::vec3 getMatrial(glm::vec4 weight);//获取材质,输入权重
		
		void cacluateErosion();//模拟侵蚀
		int randomInt(int minValue, int maxValue)
		{
			static std::mt19937 generator{
				std::random_device{}()
			};

			std::uniform_int_distribution<int> distribution(
				minValue,
				maxValue
			);

			return distribution(generator);
		}
		void Erosion(WaterDrop &water);//用于迭代的函数,正常用cacluate就行

		uint32_t getVertexIndex(glm::vec2 pos);
		bool isOutOfTerrain(const glm::vec2& position) const;//是否出地图了
		void changeHeightAround(const glm::vec2& position,float heightChange);

		float sampleHeight(const glm::vec2& position);//四点取样获取新的高度

		void threadRunErosion();//线程跑腐蚀
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