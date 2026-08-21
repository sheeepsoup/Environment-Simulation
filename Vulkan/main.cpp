//后面着色器换成slang



#include<SDL3/SDL.h>
#include<SDL3/SDL_vulkan.h>
#include<vulkan/vulkan.h>
#include<cstdlib>
#include<iostream>
#include<stdexcept>
#include"lve_windows.h"
#include"lve_pipeline.h"
#include"lve_device.h"
#include"lve_swapChain.h"
#include"lve_renderPass.h"
#include"lve_renderer.h"
#include"lve_model.h"
#include"lve_uniform.h"
#include"lve_camera.h"
#include"lve_compute.h"
#include <set>
#include<vector>
//----------------------------------------------------------------------------------------
//本地无限地形生成开关
bool unlimitedArea = false;
//----------------------------------------------------------------------------------------
lve::LveWindows win(1366,768,"从零开始的vulkan生活");//窗口
lve::Lvepipeline pipeLine("shader/simple_shader.vert.spv", "shader/simple_shader.frag.spv");
lve::LveDevice device;
lve::LveSwapChain swapChain;
lve::LveRenderPass renderPass;
lve::LveRenderer renderer;
lve::LveModel model;
lve::LveUniform uniform;
lve::LveCamera camera;
lve::LveCompute compute(device, "shader/compute.comp.spv");

uint32_t currentFrame = 0;//当前帧




#pragma region 消息回调
VkDebugUtilsMessengerEXT callback;
//回调函数
static VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "error: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

#pragma endregion

void clean() {
	vkDeviceWaitIdle(device.getDevice());
	pipeLine.clean(device.getDevice());
	swapChain.cleanupSwapChain(device.getDevice());
	renderPass.clean(device.getDevice());
	renderer.clean(device.getDevice());
	win.cleanSurface(device.getInstance());
	model.clean(device.getDevice());
	compute.clean();
	uniform.clean(device.getDevice(),renderer.getMaxFramesInFlight());
	auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(device.getInstance(), "vkDestroyDebugUtilsMessengerEXT");
	if (vkDestroyDebugUtilsMessengerEXT) {
		vkDestroyDebugUtilsMessengerEXT(device.getInstance(), callback, nullptr);
	}
	device.clean(device.getDevice(), device.getInstance());
}




int main() {
	//初始化

	
	//3.创建实例


	device.createInstance();
	//创建回调
	// 创建回调
	if (enableValidationLayers) {
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugUtilsCallback;  // 你的回调函数
		debugCreateInfo.pUserData = nullptr;  // 可传递自定义数据

		// 加载创建函数
		auto vkCreateDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(device.getInstance(), "vkCreateDebugUtilsMessengerEXT");

		if (vkCreateDebugUtilsMessengerEXT) {
			VkResult debugResult = vkCreateDebugUtilsMessengerEXT(device.getInstance(), &debugCreateInfo, nullptr, &callback);
			if (debugResult != VK_SUCCESS) {
				std::cout << "create callback error!! error code: " << debugResult << std::endl;
			}
		}
		else {
			std::cout << "unable load vkCreateDebugUtilsMessengerEXT" << std::endl;
		}
	}

	//创建表面
	win.createWindowSurface(device.getInstance(), win.win);
	device.pickPhysicalDevice();//选择物理设备
	//创建逻辑设备
	//创建队列
	device.createQueueFamiliesIndices(win);
	//创建逻辑设备
	device.createLogicalDevice();
	//检索列队句柄
	device.createArrHandle();
	
	//创建交换链
	swapChain.createSwapChain( device,win);
	
	

	//前提都设置好了---------------------------------------------------------------------------开始整画面

	pipeLine.initPipeline();//初始化管线,设置视口等信息
	device.createCommandPool(swapChain.getQueueFamilyIndices_what(0));//创建命令池
	//深度缓冲
	swapChain.createDepthResources(device);
	



	//渲染过程----------------------------------
	pipeLine.createShader(device.getDevice());//创建着色器模块


	//初始化地形
	model.processArea(114514);


	//创建计算着色器
	VkDeviceSize computeBufferSize = sizeof(int32_t) * model.heightData.size();
	compute.init(renderer.getMaxFramesInFlight(), computeBufferSize);
	const float SCALE = 10000.0f;
	std::vector<int32_t> heightUint(model.heightData.size());//高度数据
	std::vector<uint32_t> flowUint(model.heightData.size(), 0);//流量数据
	for (size_t i = 0; i < model.heightData.size(); i++) {
		heightUint[i] = static_cast<int32_t>(model.heightData[i] * SCALE + 0.5f);
	}
	for (int i = 0;i < 1;i++) {

		//计算地形
		compute.updateStorageBuffer(currentFrame, heightUint.data(), computeBufferSize);
		//提交一次计算
		VkCommandBuffer computeCmdBuf;
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		vkAllocateCommandBuffers(device.getDevice(), &allocInfo, &computeCmdBuf);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(computeCmdBuf, &beginInfo);
		compute.recordComputeCommands(computeCmdBuf, 0, model.mapVertexCount); // 只跑一次，用第0帧的 descriptor
		vkEndCommandBuffer(computeCmdBuf);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &computeCmdBuf;

		VkFence computeFence;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		vkCreateFence(device.getDevice(), &fenceInfo, nullptr, &computeFence);

		vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, computeFence);
		vkWaitForFences(device.getDevice(), 1, &computeFence, VK_TRUE, UINT64_MAX);
		vkDestroyFence(device.getDevice(), computeFence, nullptr);
		vkFreeCommandBuffers(device.getDevice(), device.getCommandPool(), 1, &computeCmdBuf);
		//计算完毕拷回来
		memcpy(heightUint.data(), compute.getMappedData(currentFrame), computeBufferSize);
		for (size_t i = 0; i < model.heightData.size(); i++) {
			model.heightData[i] = static_cast<float>(heightUint[i]) / SCALE;
		}

		memcpy(flowUint.data(),compute.getFlowMappedData(0),sizeof(uint32_t)* flowUint.size());
		uint32_t maxFlow = *std::max_element(flowUint.begin(), flowUint.end());

		for (size_t i = 0; i < flowUint.size(); i++) {
			float flow = std::log1p(static_cast<float>(flowUint[i]));
			float maxValue = std::log1p(static_cast<float>(maxFlow));
			model.vertices[i].flow = maxValue > 0.0f ? flow / maxValue : 0.0f;
		}
		//更新高度
		for (int i = 0; i < model.vertices.size(); i++) {
			model.vertices[i].pos.z = model.heightData[i];
		}

	}
	// 重新计算法线
	model.calculateNormal(); //

	//放大地形
	model.SetModelSize(2);

	//[2选1]具体区别看model.h
	//model.createVertexBuffer(device);//创建顶点缓冲区
	model.createVertexBufferWithStaging(device);//创建顶点缓冲区,使用staging buffer
	model.createIndexBufferWithStaging(device);//创建索引缓冲区


	pipeLine.distritbutePipeline();//分配管线

	//创建布局
	uniform.createDescriptorSetLayout(device.getDevice());
	uint32_t MAX_FLIT_FAMES = renderer.getMaxFramesInFlight();
	//创建UBO
	uniform.createUniformBuffer(MAX_FLIT_FAMES, device);
	//创建描述符池
	uniform.createDescriptorPool(MAX_FLIT_FAMES, device.getDevice());
	//分配并更新描述符集合
	uniform.createDescriptorSets(MAX_FLIT_FAMES, device.getDevice());

	//管线布局
	pipeLine.createPipelineLayout(device.getDevice(),uniform.getDescriptorSetLayout());//创建管线布局



	renderPass.createRenderPass(swapChain.getSwapChainSurfaceFormat(), device.findDepthFormat(),device.getDevice());//创建渲染通道



	//创建图像管线--------------------------------------------------


	pipeLine.createOthers();//创建其他管线相关信息,如视口,裁切矩形等

	pipeLine.createpipeline(renderPass.getRenderPass(), device.getDevice(),renderer.getGraphicsPipeline());//创建图像管线

	//绘制部分+--------------------------------------------------------------------
	//创建帧缓冲对象

	swapChain.createFrameBuffer(device, renderPass.getRenderPass());



	renderer.createCommandBuffers(device.getDevice(), device.getCommandPool(), swapChain.getSwapChainImageCount());//创建命令缓冲区)


	renderer.createSignalSemaphore(device.getDevice(), swapChain.getSwapChainImageCount());//创建信号量

	//初始化摄像机
	camera.setViewDirection(
		glm::vec3{ 2.0f, 2.0f, 80.0f },   // 摄像机位置
		glm::vec3{ -1.0f, -1.0f, -1.0f }, // 摄像机方向
		glm::vec3{ 0.0f, 0.0f, 1.0f });   // Z 轴向上

	camera.setPerspectiveProjection(
		glm::radians(45.0f),
		swapChain.getSwapChainExtent().width / static_cast<float>(swapChain.getSwapChainExtent().height),
		0.1f,//近裁截面
		300.0f);//远裁截面


	SDL_Event event;
	SDL_SetWindowRelativeMouseMode(win.win, true);//启用相对鼠标模式
	Uint64 lastTime = SDL_GetTicks();
	glm::mat4 modelMatrix{ 1.0f };

	modelMatrix = glm::translate(modelMatrix,glm::vec3{ 0.0f, 0.0f, 0.0f });

	modelMatrix = glm::rotate(modelMatrix,glm::radians(45.0f),glm::vec3{ 0.0f, 0.0f, 1.0f });

	modelMatrix = glm::scale(modelMatrix,glm::vec3{ 1.0f, 1.0f, 1.0f });




	while (1) {
		const Uint64 currentTime = SDL_GetTicks();

		const float dealtTime =
			static_cast<float>(currentTime - lastTime) / 1000.0f;
		lastTime = SDL_GetTicks();
		while (SDL_PollEvent(&event)) {//处理事件
			if (event.type == SDL_EVENT_QUIT) {
				clean();
				return EXIT_SUCCESS;
			}
			if (event.type == SDL_EVENT_MOUSE_MOTION) {
				camera.rotate(
					event.motion.xrel,
					event.motion.yrel);
			}
		}
		const bool* keyboardState = SDL_GetKeyboardState(nullptr);

		if (keyboardState[SDL_SCANCODE_W]) {
			camera.forward_and_behind(true, dealtTime);
		}
		if (keyboardState[SDL_SCANCODE_S]) {
			camera.forward_and_behind(false, dealtTime);
		}
		if (keyboardState[SDL_SCANCODE_A]) {
			camera.right_and_left(false, dealtTime);
		}
		if (keyboardState[SDL_SCANCODE_D]) {
			camera.right_and_left(true, dealtTime);
		}
		if (keyboardState[SDL_SCANCODE_SPACE]) {
			camera.up_and_down(true, dealtTime);
		}
		if (keyboardState[SDL_SCANCODE_LSHIFT]) {
			camera.up_and_down(false, dealtTime);
		}
		if (keyboardState[SDL_SCANCODE_ESCAPE]) {
			clean();
			return EXIT_SUCCESS;
		}
		
		renderer.run(device.getDevice(), swapChain, device.getGraphicsQueue(), device.getPresentQueue(),
			currentFrame, renderPass.getRenderPass(),model,uniform.getDescriptorSets(),pipeLine.getPipelineLayout(),
			uniform, modelMatrix,camera.getView(),camera.getProjection(),compute,camera.getPos());
		

	}

	system("pause");
	clean();


	return EXIT_SUCCESS;

}
