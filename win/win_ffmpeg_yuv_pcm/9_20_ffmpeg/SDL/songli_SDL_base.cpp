#include <stdio.h>

extern "C"
{
#include "SDL2/SDL.h"
};

const int bpp = 12;
//显示屏幕宽和高设置
int screen_w = 640, screen_h = 360;

const int pixel_w = 640, pixel_h = 360;

unsigned char buffer[pixel_w * pixel_h * bpp / 8];

int main(int argc, char* argv[])
{
	//初始化SDL 视频的
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		printf("Could not initializes  the subsystems specified by \c flags\n");
		system("puase");
		return -1;
	}

	SDL_Window * screen;
	//SDL 2.0 Support for multiple windows
	screen = SDL_CreateWindow("songli video play 1.0",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!screen)
	{
		printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
		return -1;
	}



	//
	SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

	Uint32 pixformat = 0;
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)

	pixformat = SDL_PIXELFORMAT_IYUV;

	SDL_Texture * sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat,
		SDL_TEXTUREACCESS_STREAMING, pixel_w, pixel_h);


	FILE *fp = NULL;

	fp = fopen("./SDL/sintel_640_360.yuv", "rb+");

	if (!fp)
	{
		printf("cannot open this file\n");
		return -1;
	}

	SDL_Rect sdlRect;

	while (1)
	{
		if (fread(buffer, 1, pixel_w * pixel_h * bpp / 8, fp) !=
			(pixel_h * pixel_w * bpp / 8))
		{
			//Loop
			fseek(fp, 0, SEEK_SET);
			fread(buffer, 1, pixel_h * pixel_w * bpp / 8, fp);
		}
		// SDL_UpdateTexture(SDL_Texture * texture,
		//const SDL_Rect * rect,
			//const void *pixels, int pitch);
		SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

		//设置边框
		sdlRect.x = /*1*/0;
		sdlRect.y = /*1*/0;
		sdlRect.w = screen_w  /*- 20*/;
		sdlRect.h = screen_h /*- 20*/;

		SDL_RenderClear(sdlRenderer);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
		SDL_RenderPresent(sdlRenderer);

		// Delay 40 ms;
		SDL_Delay(40);
		
	}

	SDL_Quit();
	system("pause");
	return 0;
}