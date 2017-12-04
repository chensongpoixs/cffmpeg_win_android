// Author  : songli on 2017/10/24 0024.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#include "FfmpegMusic.h"

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


//==========   ffmpeg pcm end==================

int createFFmepg(int *rate, int *channel)
{
    //注册解码器
    av_register_all();

    //初始化封装格式
    avformat_network_init();

    //查找封装格式的
    pFormatCtx = avformat_alloc_context();
    //文件路径
    const char *input = "/sdcard/input.mp3";

    //打开文件
    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0)
    {
        LOGE("%s","打开输入视频文件失败");
        return -1;
    }

    //查找文件的信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        LOGE("%s","获取视频信息失败");
        return -1;
    }

    //获取到音频的索引
    audioindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
    {
        //判断是否是音频索引
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            LOGE("  找到音频id %d", pFormatCtx->streams[i]->codec->codec_type);
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
    LOGE("获取视频编码 %p",pCodec);
    if (!pCodec)
    {
        LOGE("%s", "Didn't find AVCodec .\n");
        return -1;
    }

    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        LOGE("%s", "Couldn't open stream codec .\n");
        return -1;
    }


    //packet内存
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));
    //信息
    av_dump_format(pFormatCtx, 0, input, 0);
    //申请内存frame  像素数据
    pFrame = av_frame_alloc();


    //mp3 -> pcm 抽样的数据    转换器
    swrContext = swr_alloc();

    //开辟缓冲区的      抽样
    out_buff = (uint8_t *)av_malloc(44100 * 2);

    //16位  输出采样位数
    out_format = AV_SAMPLE_FMT_S16;
    //通道数
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
    //输出的采样率必须与输入相同
    out_smaple_rate = pCodecCtx->sample_rate;
    //转换格式
    swr_alloc_set_opts(swrContext, out_ch_layout, out_format, out_smaple_rate,
                       pCodecCtx->channel_layout, pCodecCtx->sample_fmt,
                       pCodecCtx->sample_rate, NULL, NULL
    );

    //初始化采样
    swr_init(swrContext);


    //采样
    *rate = pCodecCtx->sample_rate;
    //通道数据
    *channel = pCodecCtx->channels;

    out_buff = (uint8_t *) av_malloc(44100 * 2);
    frameindex = 0;
    LOGE("ffmpeg初始化完成");
    return 0;
}

void getPcm(void **pcm, size_t *pcm_size)
{
    //获取到通道数
    out_chananer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //读取一帧的数据
    while (av_read_frame(pFormatCtx, packet) >= 0)
    {
        //判断是否是音频的索引
        if (packet->stream_index == audioindex)
        {
            //解码器
            ret = avcodec_decode_audio4(pCodecCtx,
                                        pFrame,
                                        &got_frame_ptr,
                                        packet);
            if (ret < 0)
            {
                LOGE("Didn't audio Codec Error.");
                LOGE("解码");
            }

            if (got_frame_ptr)
            {
                //==============================================================
                swr_convert(swrContext, &out_buff, 44100 * 2,
                            (const uint8_t**)pFrame->data,
                            pFrame->nb_samples);
                //缓冲区的大小
                int size = av_samples_get_buffer_size(NULL,
                                                      out_chananer_nb,
                                                      pFrame->nb_samples,
                                                      AV_SAMPLE_FMT_S16, 1);
                //===============================================================
                LOGE("Decodec stream freameindex:%d\n", frameindex);
                frameindex++;

                //数据的赋值
                *pcm = out_buff;

                *pcm_size = (size_t)size;
                break;
            }
        }
    }
}

//释放内存
void realseFFmpeg()
{

    av_free_packet(packet);
    //缓冲区
    av_free(out_buff);
    swr_free(&swrContext);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    //关闭文件
    avformat_close_input(&pFormatCtx);

}