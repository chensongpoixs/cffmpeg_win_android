#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswresample\swresample.h"
#include "libswscale\swscale.h"
}





int main(int argc, char* argv[])
{
	//============ ffmpeg pcm ==================
	//封装格式的上下文
	AVFormatContext		*pFormatCtx;
	int					i, audioindex;
	AVCodecContext		*pCodecCtx;
	AVCodec				*pCodec;
	AVPacket			*packet;
	AVFrame				*pFrame;
	struct SwrContext	*swrContext;
	uint8_t 			*out_buff;  //pcm缓冲区
	int					out_smaple_rate; //音频的采样
	//采样的位数 
	enum AVSampleFormat 	out_format;

	//通道数
	int					out_chananer_nb;
	int					ret, frameindex;
	int					got_frame_ptr;

	//输入文件路径
	char* filepath = "songli.mp3";

	//注册所有组件
	av_register_all();

	//初始化封装格式
	avformat_network_init();


	//查找封装格式的
	pFormatCtx = avformat_alloc_context();

	
	//打开文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("Couldn't open file stream.\n");
		return -1;
	}

	//查找文件的信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find file Stream information.\n");
		return -1;
	}

	//获取到音频的索引
	audioindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		//判断是否是音频索引
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioindex = i;
			break;
		}
	}

	if (audioindex == -1)
	{
		printf("Couldn' audio find stream.codec .\n");
		return -1;
	}

	//获取mp3解码器
	pCodecCtx = pFormatCtx->streams[audioindex]->codec;

	//查找音频的解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if (!pCodec)
	{
		printf("%s", "Didn't find AVCodec .\n");
		return -1;
	}

	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("%s", "Couldn't open stream codec .\n");
		return -1;
	}


	//packet内存
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//信息
	av_dump_format(pFormatCtx, 0, filepath, 0);
	//申请内存frame
	pFrame = av_frame_alloc();


	//pcm 抽样的数据
	swrContext = swr_alloc();

	//开辟缓冲区的      抽样
	out_buff = (uint8_t *)av_malloc(44100 * 2); 

	//16位
	out_format = AV_SAMPLE_FMT_S16;
	//通道数
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
	out_smaple_rate = pCodecCtx->sample_rate;
	
	swr_alloc_set_opts(swrContext, out_ch_layout, out_format, out_smaple_rate,
		pCodecCtx->channel_layout, pCodecCtx->sample_fmt, 
		pCodecCtx->sample_rate, NULL, NULL
		);     

	//初始化采样
	swr_init(swrContext);

	//获取到通道数
	out_chananer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);


	//输出文件
	FILE *fp_pcm = fopen("songli.pcm", "wb+");

	frameindex = 0;
	//读取一帧的数据
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		//判断是否是音频的索引
		if (packet->stream_index == audioindex)
		{
			//解码器
			ret = avcodec_decode_audio4(pCodecCtx, pFrame, &got_frame_ptr, packet);
			if (ret < 0)
			{
				printf("Didn't audio Codec Error.\n");
				return -1;
			}

			if (got_frame_ptr)
			{
				//==============================================================
				swr_convert(swrContext, &out_buff, 44100 * 2,
					(const uint8_t**)pFrame->data, pFrame->nb_samples);

				int size = av_samples_get_buffer_size(NULL,
					out_chananer_nb, pFrame->nb_samples, AV_SAMPLE_FMT_S16, 1);
				//===============================================================
				printf("Decodec stream freameindex:%d\n", frameindex);
				frameindex++;
				fwrite(out_buff, 1, size, fp_pcm);
			}
		}
		av_free_packet(packet);
	}


	fclose(fp_pcm);
	swr_free(&swrContext);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	//关闭文件
	avformat_close_input(&pFormatCtx);

	system("pause");
	return EXIT_SUCCESS;
}
