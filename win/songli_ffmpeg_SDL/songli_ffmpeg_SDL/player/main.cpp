extern "C" {

#include <libavcodec\avcodec.h>
#include <libavformat\avformat.h>
#include <libswscale\swscale.h>
#include <libswresample\swresample.h>
#include "sdl\SDL.h"
#include "sdl\SDL_thread.h"
}



#include <iostream>

#include "PacketQueue.h"
#include "Audio.h"
#include "Media.h"
#include "VideoDisplay.h"
using namespace std;

bool quit = false;

int main(int argv, char* argc[])
{
	//注册解码器
	av_register_all();
	//初始化网络
	avformat_network_init();
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	//char* filename = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
	char *filename = "rtsp://120.76.194.244:1935/vod/sample.mp4";
	//char* filename = "rtmp://47.93.31.88:1935/live/songli";
	//init操作 创建 Audio 和 Video 对象
	MediaState media(filename);
	//注册ffmpeg   ----init----
	if (media.openInput())
		SDL_CreateThread(decode_thread, "", &media); // 创建解码线程，读取packet到队列中缓存
	//init sdl  audio 
	media.audio->audio_play(); // create audio thread

	media.video->video_play(&media); // create video thread

	AVStream *audio_stream = media.pFormatCtx->streams[media.audio->stream_index];
	AVStream *video_stream = media.pFormatCtx->streams[media.video->stream_index];

	double audio_duration = audio_stream->duration * av_q2d(audio_stream->time_base);
	double video_duration = video_stream->duration * av_q2d(video_stream->time_base);

	cout << "audio时长：" << audio_duration << endl;
	cout << "video时长：" << video_duration << endl;

	SDL_Event event;
	while (true) // SDL event loop
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case FF_QUIT_EVENT:
		case SDL_QUIT:
			quit = 1;
			SDL_Quit();

			return 0;
			break;

		case FF_REFRESH_EVENT:
			video_refresh_timer(&media);
			break;

		default:
			break;
		}
	}

	getchar();
	return 0;
}