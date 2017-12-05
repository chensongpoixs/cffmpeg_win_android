#define _CRT_SECURE_NO_WARNINGS
#include "../ffmpeg.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


using namespace std;  //使用   标准的命名空间

static AVFormatContext *fmt_ctx;
static AVCodecContext *dec_ctx;
AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx;
AVFilterGraph *filter_graph;
static int audio_stream_index = -1;

static int open_input_file(const char *filename)
{
	int ret;
	AVCodec *dec;

	if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return ret;
	}

	if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
		return ret;
	}

	/* select the audio stream */
	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot find an audio stream in the input file\n");
		return ret;
	}
	audio_stream_index = ret;

	/* create decoding context */
	dec_ctx = avcodec_alloc_context3(dec);
	if (!dec_ctx)
		return AVERROR(ENOMEM);
	avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[audio_stream_index]->codecpar);
	av_opt_set_int(dec_ctx, "refcounted_frames", 1, 0);

	/* init the audio decoder */
	if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
		return ret;
	}

	return 0;
}

void Java_com_dongnao_ffmpegdemo_MainActivity_open() {
	

	int vedio_stream_idx = -1;
	//    找到视频流
	for (int i = 0; i < pContext->nb_streams; ++i) {
		printf("循环  %d", i);
		//      codec 每一个流 对应的解码上下文   codec_type 流的类型
		if (pContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			vedio_stream_idx = i;
		}
	}

	//    获取到解码器上下文
	AVCodecContext* pCodecCtx = pContext->streams[vedio_stream_idx]->codec;

	//    解码器
	AVCodec *pCodex = avcodec_find_decoder(pCodecCtx->codec_id);
	//    ffempg版本升级
	if (avcodec_open2(pCodecCtx, pCodex, NULL) < 0) {
		printf("解码失败");
		return;
	}
	//    分配内存   malloc  AVPacket   1   2
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//    初始化结构体
	av_init_packet(packet);
	//还是不够
	AVFrame *frame = av_frame_alloc();
	//    声明一个yuvframe
	AVFrame *yuvFrame = av_frame_alloc();
	//    给yuvframe  的缓冲区 初始化

	uint8_t  *out_buffer = (uint8_t *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));

	int re = avpicture_fill((AVPicture *)yuvFrame, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	printf("宽 %d  高 %d", pCodecCtx->width, pCodecCtx->height);
	//    mp4   的上下文pCodecCtx->pix_fmt
	SwsContext *swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P
		, SWS_BILINEAR, NULL, NULL, NULL
	);
	int frameCount = 0;
	FILE *fp_yuv = fopen(outStr, "wb");

	//packet入参 出参对象  转换上下文
	int got_frame;
	while (av_read_frame(pContext, packet) >= 0) {
		//        节封装

		//        根据frame 进行原生绘制    bitmap  window
		avcodec_decode_video2(pCodecCtx, frame, &got_frame, packet);
		//   frame  的数据拿到   视频像素数据 yuv   三个rgb    r   g  b   数据量大   三个通道
		//        r  g  b  1824年    yuv 1970
		printf("解码%d  ", frameCount++);
		if (got_frame > 0) {
			sws_scale(swsContext, (const uint8_t *const *)frame->data, frame->linesize, 0, frame->height, yuvFrame->data,
				yuvFrame->linesize
			);
			int y_size = pCodecCtx->width * pCodecCtx->height;
			//        y 亮度信息写完了
			fwrite(yuvFrame->data[0], 1, y_size, fp_yuv);
			fwrite(yuvFrame->data[1], 1, y_size / 4, fp_yuv);
			fwrite(yuvFrame->data[2], 1, y_size / 4, fp_yuv);
		}
		av_free_packet(packet);
	}
	fclose(fp_yuv);
	av_frame_free(&frame);
	av_frame_free(&yuvFrame);
	avcodec_close(pCodecCtx);
	avformat_free_context(pContext);
	
}
int main(void)
{
	char* input = "D:\FILE\input.mp4";
	char* outputstr = "D:\FILE\output.yuv";
	Java_com_dongnao_ffmpegdemo_MainActivity_open(&input, &outputstr);
	

	system("pause");
	return 0;
}

