
#include "Media.h"
#include <iostream>

extern "C"{
#include <libavutil/time.h>
}
extern bool quit;

MediaState::MediaState(char* input_file)
	:filename(input_file) //给文件赋值操作
{
	pFormatCtx = nullptr;
	audio = new AudioState();   //null Queue  packet

	video = new VideoState();   //Queue  packet
	//quit = false;
}

MediaState::~MediaState()
{
	if(audio)
		delete audio;

	if (video)
		delete video;
}
//ffmpeg init        audio 和vedio流的索引的查找
bool MediaState::openInput()
{
	// Open input file
	if (avformat_open_input(&pFormatCtx, filename, nullptr, nullptr) < 0)
		return false;

	if (avformat_find_stream_info(pFormatCtx, nullptr) < 0)
		return false;

	// Output the stream info to standard 
	av_dump_format(pFormatCtx, 0, filename, 0);

	//音视频的流的索引
	for (uint32_t i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio->stream_index < 0)
			audio->stream_index = i;

		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video->stream_index < 0)
			video->stream_index = i;
	}

	if (audio->stream_index < 0 || video->stream_index < 0)
		return false;
	//=======================vedio audio codec start====================================
	// Fill audio state
	AVCodec *pCodec = avcodec_find_decoder(pFormatCtx->streams[audio->stream_index]->codec->codec_id);
	if (!pCodec)
		return false;

	audio->stream = pFormatCtx->streams[audio->stream_index];

	audio->audio_ctx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(audio->audio_ctx, pFormatCtx->streams[audio->stream_index]->codec) != 0)
		return false;

	avcodec_open2(audio->audio_ctx, pCodec, nullptr);

	// Fill video state
	AVCodec *pVCodec = avcodec_find_decoder(pFormatCtx->streams[video->stream_index]->codec->codec_id);
	if (!pVCodec)
		return false;

	video->stream = pFormatCtx->streams[video->stream_index];

	video->video_ctx = avcodec_alloc_context3(pVCodec);
	if (avcodec_copy_context(video->video_ctx, pFormatCtx->streams[video->stream_index]->codec) != 0)
		return false;

	avcodec_open2(video->video_ctx, pVCodec, nullptr);
	//=======================vedio audio codec end====================================
	//得到当前的时间
	video->frame_timer = static_cast<double>(av_gettime()) / 1000000.0; 
	video->frame_last_delay = 40e-3;

	return true;
}

//main解码线程的操作
int decode_thread(void *data)
{
	MediaState *media = (MediaState*)data;

	AVPacket *packet = av_packet_alloc();

	while (true)
	{
		// read 一帧的packet数据
		int ret = av_read_frame(media->pFormatCtx, packet);
		//error 的处理
		if (ret < 0)
		{
			if (ret == AVERROR_EOF)
				break;
			if (media->pFormatCtx->pb->error == 0) // No error,wait for user input
			{
				SDL_Delay(100);
				continue;
			}
			else
				break;
		}
		//解码的操作 audio和Vedio索引   放到对应队列中
		if (packet->stream_index == media->audio->stream_index) // audio stream
		{
			media->audio->audioq.enQueue(packet);
			av_packet_unref(packet);
		}		
		else if (packet->stream_index == media->video->stream_index) // video stream
		{
			media->video->videoq->enQueue(packet);
			av_packet_unref(packet);
		}		
		else
			av_packet_unref(packet);
	}

	av_packet_free(&packet);

	return 0;
}