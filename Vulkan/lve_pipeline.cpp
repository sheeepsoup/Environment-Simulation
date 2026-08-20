#include"lve_pipeline.h"

namespace lve {
	std::vector<char> Lvepipeline::readFile(const std::string& filename) {

		std::ifstream file{ filename, std::ios::ate | std::ios::binary };
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filename);
		}
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	};
	void Lvepipeline::createGrasphicsPipeline(const std::string& vertexLine, const std::string& fragmentLine) {

		auto vertShaderCode = readFile(vertexLine);
		auto fragShaderCode = readFile(fragmentLine);
		fragmentCode = fragShaderCode;
		vertexCode = vertShaderCode;
		std::cout << "Vertex Shader Code Size: " << vertShaderCode.size() << std::endl;
		std::cout << "Fragment Shader Code Size: " << fragShaderCode.size() << std::endl;
	}
	Lvepipeline::Lvepipeline(const std::string& vertexLine, const std::string& fragmentLine) {
		createGrasphicsPipeline(vertexLine, fragmentLine);
	}
	std::vector<char>& Lvepipeline::getVertexCode() {
		return vertexCode;
	}
	std::vector<char>& Lvepipeline::getFragmentCode() {
		return fragmentCode;
	}
	void Lvepipeline::initPipeline() {
		//启用动态视口和裁切矩形
	//视口

		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;//后面记得管线里塞动态的

		//光栅化器

		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;//禁用深度钳位，超出范围的片元会被丢弃
		rasterizer.rasterizerDiscardEnable = VK_FALSE;//是否丢弃所有光栅化输出[true就是跳过光栅化]
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;//模式是填充满
		rasterizer.lineWidth = 1.0f;//线框模式的时候的边长像素[只有线框模式才会发挥作用]
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;//剔除类型:背面剔除
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;//逆时针顶点为正面
		rasterizer.depthBiasEnable = VK_FALSE;//关闭深度偏移
		rasterizer.depthBiasConstantFactor = 0.0f; //常量偏移值[如果开启的话]
		rasterizer.depthBiasClamp = 0.0f; //偏移最大限制
		rasterizer.depthBiasSlopeFactor = 0.0f; //斜率偏移值

		//多重采样[一般用于实现抗锯齿]

		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;//现在目前禁用,后面加上
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//颜色混合[跟帧缓冲有关]

		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;//混合模式是否开启[下面都是和混合模式有关的]
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;			//到时候自己看着改
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;	//到时候自己看着改
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;							//到时候自己看着改
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;					//到时候自己看着改
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;				//到时候自己看着改
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;							//到时候自己看着改


		//颜色全局状态[依旧关闭]

		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		//深度缓冲测试开启
		depthStencil.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;

		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};
	}
	void Lvepipeline::createPipelineLayout(VkDevice device,VkDescriptorSetLayout descriptorSetLayout) {
		//管线信息
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1; //描述符的数量
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; //对应地址
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}
	void Lvepipeline::createpipeline(VkRenderPass renderPass,VkDevice device, VkPipeline &graphicsPipeline) {
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;//着色器数量
		pipelineInfo.pStages = shaderStages;//着色器信息
		pipelineInfo.pVertexInputState = &vertexInputInfo;//顶点输入
		pipelineInfo.pInputAssemblyState = &inputAssembly;//输入汇编
		pipelineInfo.pViewportState = &viewportState;//视图信息
		pipelineInfo.pRasterizationState = &rasterizer;//光栅化信息
		pipelineInfo.pMultisampleState = &multisampling;//多重采样
		pipelineInfo.pDepthStencilState = &depthStencil; //深度信息
		pipelineInfo.pColorBlendState = &colorBlending;//颜色混合模式
		pipelineInfo.pDynamicState = &dynamicState;//动态状态
		pipelineInfo.layout = pipelineLayout;//管线布局,他是一个vulkan句柄不是指针
		pipelineInfo.renderPass = renderPass;//渲染通道
		pipelineInfo.subpass = 0;//使用第0个子过程
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;//管线继承方面的,指向一个已经创建好的直接复制[默认用NullHandle来创建]
		pipelineInfo.basePipelineIndex = -1;//复制的时候复制第几个管线 [-1表示自己创建]


		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {//创建图像管线
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}
	void Lvepipeline::distritbutePipeline() {

		//分配管线
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;//类型
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;//指定是顶点着色器阶段
		vertShaderStageInfo.module = vertexShader;
		vertShaderStageInfo.pName = "main";//指定入口函数名

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;//指定是片段着色器阶段
		fragShaderStageInfo.module = fragmentShader;
		fragShaderStageInfo.pName = "main";//指定入口函数名

		shaderStages[0] = vertShaderStageInfo;
		shaderStages[1] = fragShaderStageInfo;
	}
	void Lvepipeline::createOthers() {

		//动态状态

		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		//顶点输入
		bindingDescription = LveModel::Vertex::getBindingDescription();//获取绑定描述
		attributeDescriptions = LveModel::Vertex::getAttributeDescriptions();//获取属性描述
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;//顶点输入结构体
		vertexInputInfo.vertexBindingDescriptionCount = 1;//要绑定的顶点数
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;//绑定描述
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());//顶点属性描述数量
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();//顶点属性描述

		//输入汇编{怎么画}

		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;//拓补类型:每3个点画一个三角形
		inputAssembly.primitiveRestartEnable = VK_FALSE;//图元重启禁用
	}
	VkShaderModule Lvepipeline::createShaderModule(VkDevice device, const std::vector<char>& code) {

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}
	void Lvepipeline::createShader(VkDevice device){
		vertexShader = createShaderModule(device, getVertexCode());
		fragmentShader = createShaderModule(device,getFragmentCode());
	}
	void Lvepipeline::clean(VkDevice device) {
		vkDestroyShaderModule(device, fragmentShader, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}
