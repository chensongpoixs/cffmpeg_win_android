#include <stdio.h>

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
};


int main(int argc, char* argv[])
{
	
	AVFormatContext	*pFormatCtx;
	int				i, videoindex;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame	*pFrame, *pFrameYUV;
	uint8_t *out_buffer;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	//输入文件路径
	char filepath[] = "Titanic.ts";

	int frame_cnt;

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		printf("Couldn't open input stream.\n");
		getchar();
		//system("puase");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		printf("Couldn't find stream information.\n");
		return -1;
	}
	//获取视频的流的id
	videoindex = -1;
	for (i = 0; i<pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		printf("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		printf("Codec not found.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
		printf("Could not open codec.\n");
		return -1;
	}
	//==============================================================
	/*
	* 在此处添加输出视频信息的代码
	* 取自于pFormatCtx，使用fprintf()
	*/
	//获取视频的时长
	printf("duration : %d\n", pFormatCtx->duration);
	//获取视频的bit_rate
	printf("bit_rate : %d\n", pFormatCtx->bit_rate);
	//获取封装格式
	printf("视频封装格式 : %s\n", pFormatCtx->iformat->name);

	//获取视频的宽高   视频的 流一般是0
	printf("宽：%d， 高：%d\n", pFormatCtx->streams[videoindex]->codec->width, pFormatCtx->streams[videoindex]->codec->height );
	//得到是封装类
	printf("class name = %s\n", pFormatCtx->av_class->class_name);


	printf("version = %d\n", pFormatCtx->av_class->version);
	//获取到输入文件文件名
	printf("filename = %s\n", pFormatCtx->filename);

	printf(" ctx-flags = %d\n", pFormatCtx->ctx_flags);

	printf("nb_streams = %d\n", pFormatCtx->nb_chapters);
	pFormatCtx->start_time_realtime = (int64_t) 90;

	AVStream *st;
	//得到PTS 时间
	st = pFormatCtx->streams[videoindex];
	double time = st->duration * av_q2d(st->time_base);
	printf("time :%ld\n", time);
	printf("start_time_realtime = %d\n", pFormatCtx->start_time_realtime);


	printf("max_streams = %d\n", pFormatCtx->max_streams);
	//=========================================================================
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	av_dump_format(pFormatCtx, 0, filepath, 0);
	printf("-------------------------------------------------\n");
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	//视频文件h264
	FILE * fp_h264 = fopen("songli.h264", "wb+");
	//视频文件yuv
	FILE * fp_yuv = fopen("songli.yuv", "wb+");

	frame_cnt = 0;
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoindex) {
			
			/*
			* 在此处添加输出H264码流的代码
			* 取自于packet，使用fwrite()
			*/
			fwrite(packet->data, 1, packet->size, fp_h264);
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) {
				printf("Decode Error.\n");
				return -1;
			}
			if (got_picture) {
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height,
					pFrameYUV->data, pFrameYUV->linesize);
				printf("Decoded frame index: %d\n", frame_cnt);

				/*
				* 在此处添加输出YUV的代码
				* 取自于pFrameYUV，使用fwrite()
				*/
				// y的数据
				fwrite(pFrameYUV->data[0], 1, pCodecCtx->width * pCodecCtx->height, fp_yuv);

				// u v     yuv420 
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_yuv);
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->width * pCodecCtx->height / 4, fp_yuv);
				
				frame_cnt++;

			}
		}
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
	system("puase");
	return 0;
}

