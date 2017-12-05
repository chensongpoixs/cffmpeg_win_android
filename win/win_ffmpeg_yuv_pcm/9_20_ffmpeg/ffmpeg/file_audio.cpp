#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "../ffmpeg.h"

using namespace std;  //使用   标准的命名空间



void test01()
{
	//FFmpeg  
	AVFormatContext *pFormatCtx;
	int             i, videoindex;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame *pFrame, *pFrameYUV;
	AVPacket *packet;
	struct SwsContext *img_convert_ctx;
	//SDL  
	int screen_w, screen_h;
	SDL_Surface *screen;
	SDL_VideoInfo *vi;
	SDL_Overlay *bmp;
	SDL_Rect rect;

	FILE *fp_yuv;
	int ret, got_picture;
	char filepath[] = "bigbuckbunny_480x272.h265";

	av_register_all();
	avformat_network_init();

	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}
}



int ffmain(void)
{
	printf("%s", avcodec_configuration());


	system("pause");
	return 0;
}