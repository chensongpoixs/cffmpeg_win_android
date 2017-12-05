#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
#include "libswresample/swresample.h"
#include "sdl\SDL.h"
}

#define VEDIOH 0
#define PCM    0
#define TXT    1

//=======================audio =============
#define MAX_AUDIO_FRAME_SIZE 192000  
//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;


//Thread Loop constant
#define SFM_REFRESH_EVENT (SDL_USEREVENT + 1)
#define SFM_BREAK_EVENT (SDL_USEREVENT + 2)
//信息
int thread_exit = 0;

int sfp_refresh_thread(void * opaque)
{
	thread_exit = 0;
	while (!thread_exit)
	{
		//事件
		SDL_Event event;
		//设置事件
		event.type = SFM_REFRESH_EVENT;

		//推送事件
		SDL_PushEvent(&event);
		//设置等待事件
		SDL_Delay(20);
	}

	//收到退出事件的处理
	thread_exit = 0;
	//Break;
	SDL_Event event;
	//退出的
	event.type = SFM_BREAK_EVENT;

	SDL_PushEvent(&event);
	return 0;
}

void  fill_video(void *udata, Uint8 *stream, int len) {
	//SDL 2.0
	SDL_memset(stream, 0x00, len);
	if (audio_len == 0)
		return;

	len = (len>audio_len ? audio_len : len);	/*  Mix  as  much  data  as  possible  */
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}



int main(int argc, char* argv[])
{
	//================ffmpeg================
	AVFormatContext *pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrame, *pFrameYUV;
	uint8_t			*out_buffer;  //视频解码的缓冲区
	AVPacket		*packet;
	int				ret, got_picture;
	int				sum, unm = 0;
	//================audio init=================
	AVCodecContext			*audio_CodecCtx;
	AVCodec					*audio_Codec;
	SDL_AudioSpec			wanted_spec;
	AVPacket		        *audio_packet;
	AVFrame					*audio_Frame;
	int						audio_index;
	uint32_t				len = 0;
	uint8_t					*audio_out_buffer; //音频的缓冲区
	int						audio_got_picture;
	int						index = 0;
	int64_t					in_channel_layout;
	struct SwrContext		*au_convert_ctx;

	//===============SDL=====================
	int screen_w, screen_h;
	SDL_Window	*screen;
	SDL_Renderer	*sdlRenderer; //视频的
	SDL_Texture		*sdlTexture;
	SDL_Rect		sdlRect;
	SDL_Thread		*video_tid;
	SDL_Event		event;

	struct SwsContext *img_convert_ctx;

	char filepath[] = "屌丝男士.mov";

	//注册ffmpeg所有的组件
	av_register_all();

	//初始化ffmpeg视频的封装格式
	avformat_network_init();

	//获取封装格式的上下文
	pFormatCtx = avformat_alloc_context();



	
	//打开视频文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}
	//查找封装格式
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}
	av_dump_format(pFormatCtx, 0, filepath, false);

	//获取音频视频流索引 
	videoindex = -1;
	audio_index = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		//判断是否视频的索引
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;	
			//获取视频解码器
			pCodecCtx = pFormatCtx->streams[videoindex]->codec;
			//查找视频解码器
			pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
			if (videoindex == -1)
			{
				printf("Didn't find a video stream.\n");
				system("pause");
				return -1;
			}
			if (pCodec == NULL) {
				printf("Codec not found.\n");
				system("pause");
				return -1;
			}
			if (avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
				printf("Could not open codec.\n");
				system("pause");
				return -1;
			}
		} else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_index = i;  //判断是否音频的索引
			//获取解码器
			audio_CodecCtx = pFormatCtx->streams[audio_index]->codec;
			audio_Codec = avcodec_find_decoder(audio_CodecCtx->codec_id);
			if (audio_index == -1) {
				printf("Didn't find a audio stream.\n");
				system("pause");
				return -1;
			}
			if (audio_Codec == NULL) {
				printf("Codec not found.\n");
				system("pause");
				return -1;
			}
			if (avcodec_open2(audio_CodecCtx, audio_Codec, NULL)<0) {
				printf("Could not open codec.\n");
				system("pause");
				return -1;
			}
		}
	}
	
	//====================Video start========================
	//h264视频帧注册
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();

	//开辟内存
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height
	));

	//fill开辟内存
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height);

	//转yuv格式
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height,
		AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);

	//=================audio start======================
	audio_packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_init_packet(audio_packet);

	//Out Audio Param
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	//nb_samples: AAC-1024 MP3-1152
	int out_nb_samples = audio_CodecCtx->frame_size;
	//采样的位数
	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	//采样的频率
	int out_sample_rate = 44100;
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	//Out Buffer Size
	int out_buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, out_sample_fmt, 1);

	audio_out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
	audio_Frame = av_frame_alloc();
	//=================audio end======================

	//初始化SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO))
	{
		printf("Could not initialize SDL - %s\n", SDL_GetError());
		system("pause");
		return -1;
	}

	//SDL 2.0 Support for multiple windows

	//设置宽度 高度
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//创建窗口
	screen = SDL_CreateWindow("songli ffmpeg player sdl 1.0",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL);

	if (!screen)
	{
		printf("SDL: could not create window -exiting:%s\n", SDL_GetError());
		system("pause");
		return -1;
	}

	//创建Renderer
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);


	//IYUV : Y + U + V (3 planes);
	//YV12: Y + U + V (3 planes);
	sdlTexture = SDL_CreateTexture(sdlRenderer,
		SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		pCodecCtx->width, pCodecCtx->height);


	//设置屏幕宽度和高度
	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	//文件information
	av_dump_format(pFormatCtx, 0, filepath, 0);
	//给视频包开辟内存
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	//创建Thread
	video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);
	
	//==============audio sdl ====================
	//SDL_AudioSpec
	wanted_spec.freq = out_sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = out_channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = out_nb_samples;
	wanted_spec.callback = fill_video; //回调函数
	wanted_spec.userdata = audio_CodecCtx;
	if (SDL_OpenAudio(&wanted_spec, NULL)<0) {
		printf("can't open audio.\n");
		system("pause");
		return -1;
	}
	in_channel_layout = av_get_default_channel_layout(audio_CodecCtx->channels);
	//Swr

	au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx,
		out_channel_layout, out_sample_fmt, out_sample_rate,
		in_channel_layout, audio_CodecCtx->sample_fmt,
		audio_CodecCtx->sample_rate, 0, NULL);
	swr_init(au_convert_ctx);

	//Play
	SDL_PauseAudio(0);
	//==============audio sdl end====================
#if VEDIOH
	FILE *fp_h264 = fopen("songli.h264", "wb+");
	FILE *fp_yuv = fopen("songli.yuv", "wb+");
#endif

#if PCM
	FILE *fp_pcm = fopen("songli.pcm", "wb+");
#endif
#if TXT
	FILE *fp_txt = fopen("songli.txt", "wb+");
#endif
	//事件的Loop
	sum = 0;
	for (;;)
	{
		
		//Wait
		SDL_WaitEvent(&event);
		//判断事件
		if (event.type == SFM_REFRESH_EVENT)
		{
			if (av_read_frame(pFormatCtx, packet) >= 0)
			{
				//printf("-----------vidio start --------------------\n");

				//找到视频流索引
				if (packet->stream_index == videoindex)
				{
#if VEDIOH
					//写入h264 文件中
					fwrite(packet->data, 1, packet->size, fp_h264);
#endif	
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if (ret < 0)
					{
						printf("Decode Error.\n");
						system("pause");
						return -1;
					}
					if (got_picture)
					{
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
							pFrame->linesize, 0,
							pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
#if VEDIOH
						//写入yuv 文件中
						fwrite(pFrameYUV->data[0], 1, pCodecCtx->width * pCodecCtx->height, fp_yuv);

						// u v     yuv420 
						fwrite(pFrameYUV->data[1], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_yuv);
						fwrite(pFrameYUV->data[2], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_yuv);

#endif
#if TXT
						char buf[1024] = { 0 };
						sprintf(buf, "vedio num = [%4d], index:[%5d]\t pts:[%lld]\t packet size:[%d]\n", unm, sum, packet->pts, packet->size);
						fwrite(buf, 1, strlen(buf), fp_txt);
#endif
						//printf("Decoded frame index:%d\n", sum);
						sum++;
						//=============SDL=================
						SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0],
							pFrameYUV->linesize[0]);

						SDL_RenderClear(sdlRenderer);

						SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent(sdlRenderer);
					}
				}
				av_free_packet(packet);
				
				//printf("-----------vidio end --------------------\n");
			}
			else
			{
				//Exit Thread;
				thread_exit = 1;
			}
			//音频处理
			if (av_read_frame(pFormatCtx, audio_packet) >= 0) {
				//音频处理
				//printf("-----------audio start --------------------\n");
				if (audio_packet->stream_index == audio_index) {
					ret = avcodec_decode_audio4(audio_CodecCtx, audio_Frame, &audio_got_picture, audio_packet);
					if (ret < 0) {
						printf("Error in decoding audio frame.\n");
						system("pause");
						return -1;
					}
					
					if (audio_got_picture > 0) {
						swr_convert(au_convert_ctx,
							&audio_out_buffer,
							MAX_AUDIO_FRAME_SIZE,
							(const uint8_t **)audio_Frame->data,
							audio_Frame->nb_samples);
#if TXT
						char buf[1024] = { 0 };
						sprintf(buf, "audio num = [%4d], index:[%5d]\t pts:[%lld]\t packet size:[%d]\n", unm,index, audio_packet->pts, audio_packet->size);
						fwrite(buf, 1, strlen(buf), fp_txt);
#endif
#if PCM
						//Write PCM
						fwrite(audio_out_buffer, 1, out_buffer_size, fp_pcm);
#endif
						index++;
					}
					while (audio_len>0)//Wait until finish
						SDL_Delay(1);
					//Set audio buffer (PCM data)
					audio_chunk = (Uint8 *)audio_out_buffer;
					//Audio buffer length
					audio_len = out_buffer_size;
					audio_pos = audio_chunk;
				}
				av_free_packet(audio_packet);

				//printf("-----------audio end --------------------\n");
			}
			
		}
		else if (event.type == SDL_QUIT)
		{
			thread_exit = 1;
		}
		else if (event.type == SFM_BREAK_EVENT)
		{
			break;
		}	
		unm++;
	}
	swr_free(&au_convert_ctx);
	SDL_CloseAudio();//Close SDL
	SDL_Quit();

#if VEDIOH
	fclose(fp_h264);
	fclose(fp_yuv);
#endif
#if PCM
	fclose(fp_pcm);
#endif

#if TXT
	fclose(fp_txt);
#endif
	sws_freeContext(img_convert_ctx);
	SDL_Quit();
	av_frame_free(&pFrame);
	av_frame_free(&audio_Frame);
	av_free(audio_out_buffer);
	av_frame_free(&pFrameYUV);
	avcodec_close(pCodecCtx);
	avcodec_close(audio_CodecCtx);
	avformat_close_input(&pFormatCtx);
	system("pause");
	return EXIT_SUCCESS;
}