#include"lve_windows.h"

namespace lve {
    LveWindows::LveWindows(int w, int h, std::string name) {
        width = w;
        heigh = h;
        window_name = name;
		InitWindow();
    }
	void LveWindows::InitWindow() {
		SDL_Init(SDL_INIT_VIDEO);
		win = SDL_CreateWindow(window_name.c_str(),width, heigh, SDL_WINDOW_VULKAN);
	}
	LveWindows::~LveWindows() {
		SDL_DestroyWindow(win);
		SDL_Quit();
	}
	void LveWindows::createWindowSurface(const VkInstance instance,SDL_Window* win) {
		if (!SDL_Vulkan_CreateSurface(win, instance, nullptr, &surface)) {
			throw std::runtime_error("创建窗口表面失败");
		}
	};
	void LveWindows::cleanSurface(VkInstance instance) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
}