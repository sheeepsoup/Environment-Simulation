#pragma once
#include<SDL3/SDL.h>
#include<string>
#include<vulkan/vulkan.h>
#include<SDL3/SDL_vulkan.h>
#include<iostream>
namespace lve {
	class LveWindows {
	public:
		SDL_Window* win = NULL;
		LveWindows(int w, int h, std::string name);
		~LveWindows();
		void InitWindow();
		struct vec2
		{
			int w;
			int h;
		};
		vec2 get_heigh_width() { return{ width, heigh }; };
		void createWindowSurface(const VkInstance instance,SDL_Window* win);//创建表面
		void cleanSurface(VkInstance instance);
		VkSurfaceKHR getSurface() { return surface; };
	
	private:
		 VkSurfaceKHR surface;//表面
		 int heigh;
		 int width;
		std::string window_name;

	
	};
}