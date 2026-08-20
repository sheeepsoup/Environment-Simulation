#pragma once
#include<string>
#include<vulkan/vulkan.h>
#include<vector>
#include<fstream>
#include<stdexcept>
#include<iostream>
#include"lve_model.h"
#include"lve_uniform.h"
namespace lve {
	class Lvepipeline {
	public:
		Lvepipeline(const std::string& vertexLine, const std::string& fragmentLine);


		std::vector<char>& getVertexCode();//获取着色器
		std::vector<char>& getFragmentCode();
		
		void initPipeline();
		void createPipelineLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);
		void createpipeline(VkRenderPass renderPass, VkDevice device, VkPipeline &graphicsPipeline);
		void distritbutePipeline();
		void createOthers();
		void createShader(VkDevice device);//创建着色器
		VkPipelineLayout getPipelineLayout() { return pipelineLayout; };
		void clean(VkDevice device);
	private:
		VkPipelineDepthStencilStateCreateInfo depthStencil{};//深度缓冲测试
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};//动态状态参数
		static std::vector<char> readFile(const std::string& filename);
		void createGrasphicsPipeline(const std::string& vertexLine, const std::string& fragmentLine);
		//创建着色器模块
		VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
		std::vector<char> vertexCode;   //顶点着色器代码
		std::vector<char> fragmentCode; //片段着色器代码
		VkShaderModule vertexShader;//顶点着色器模块
		VkShaderModule fragmentShader;//片段着色器模块
		VkPipelineShaderStageCreateInfo shaderStages[2];//着色器阶段,存储着色器信息
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};//颜色混合[跟帧缓冲有关]
		VkPipelineColorBlendStateCreateInfo colorBlending{};//颜色全局状态[依旧关闭]
		VkPipelineLayout pipelineLayout;//管线布局
		VkGraphicsPipelineCreateInfo pipelineInfo{};//管线信息
		VkPipelineViewportStateCreateInfo viewportState{};//视口
		VkPipelineRasterizationStateCreateInfo rasterizer{};//光栅化器
		VkPipelineMultisampleStateCreateInfo multisampling{};//多重采样
		VkPipelineDynamicStateCreateInfo dynamicState{};//动态状态
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};//顶点输入
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};//输入汇编

		VkVertexInputBindingDescription
			bindingDescription{};//顶点输入绑定描述符

		std::array<VkVertexInputAttributeDescription,4> attributeDescriptions{};//顶点输入属性描述符
	};

}