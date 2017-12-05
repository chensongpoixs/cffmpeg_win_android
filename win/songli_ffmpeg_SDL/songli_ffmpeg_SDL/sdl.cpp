#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
extern "C"
{
#include "sdl/SDL.h"
}

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	//创建窗口
	SDL_Window* window = SDL_CreateWindow("songli",
		500, 500,  //宽度 ，高度
		500, 500,
		SDL_WINDOW_SHOWN);

	SDL_Event event;
	while (1)
	{
		while (SDL_PollEvent(&event))
		{
			// 值是否退出 SDL_QUIT退出
			if (event.type == SDL_QUIT)
			{
				return 0;
			}
		}
	}


	system("pause");
	return EXIT_SUCCESS;
}