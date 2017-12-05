#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
#include "libavcodec\avcodec.h"
#include "libavformat\avformat.h"
#include "libswscale\swscale.h"
}

int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrame, *pFrameYUV;
	uint8_t			*out_buffer;
	AVPacket		*packet;
	int				y_size;
	int				ret, got_picture;
	struct SwsContext	*img_convert_ctx;
	//输入文件路径
	char filepath[] = "Titanic.ts";

	int frame_cnt;
	//注册所有组件
	av_register_all();
	//初始化封装格式
	avformat_network_init();
	//获取封装格式的
	pFormatCtx = avformat_alloc_context();

	//打开文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
	{
		printf("Couldn't open input stream.\n");
		return -1;
	}

	//获取文件的信息
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		printf("Couldn't find stream information.\n");
		return -1;
	}

	videoindex = -1;
	//获取视频流的id
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		//判断是否是视频流
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}

	if (videoindex == -1)
	{
		printf("Didn't find a video stream.\n");
		return -1;
	}


	//获取视频解码器
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	//查找视频解码器
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	if (!pCodec)
	{
		printf("Codec not found.\n");
		return -1;
	}

	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		printf("Could not open codec.\n");
		return -1;
	}

	//给视频开辟内存
	pFrame = av_frame_alloc();

	//yuv开辟内存空间
	pFrameYUV = av_frame_alloc();

	//申请缓冲区
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height));

	//file缓冲区
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer,
		AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);

	//申请一个Packet空间
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	//文件information
	av_dump_format(pFormatCtx, 0, filepath, 0);

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height,
		AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);

	//视频文件
	FILE *fp_h264 = fopen("chenli.h264", "wb+");
	FILE *fp_yuv = fopen("chenli.yuv", "wb+");

	frame_cnt = 0;
	//读取一帧的数据
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		//判断是否是视频流
		if (packet->stream_index == videoindex)
		{
			//写入读取的h.264文件数据
			fwrite(packet->data, 1, packet->size, fp_h264);

			ret = avcodec_decode_video2(pCodecCtx, pFrame,
				&got_picture, packet);

			if (ret < 0)
			{
				printf("Decode Error.\n");
				return -1;
			}

			if (got_picture)
			{
				//视频数据转yuv
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);
				printf("Decoded frame index:%d\n", frame_cnt);

				//写入yuv数据
				fwrite(pFrameYUV->data[0], 1, pCodecCtx->width * pCodecCtx->height, fp_yuv);
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_yuv);
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_yuv);


				frame_cnt++;
			}
		}
		//释放packet
		av_free_packet(packet);
	}

	//关闭文件
	fclose(fp_yuv);
	fclose(fp_h264);

	sws_freeContext(img_convert_ctx);

	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);


	system("pause");
	return EXIT_SUCCESS;
}