#include <jni.h>
#include <string>




extern  "C"
{
#include <unistd.h>
#include "libswresample/swresample.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/samplefmt.h"
#include <android/native_window_jni.h>
#include "activity_songli_com_songlihlcplayer_SongliPlayer.h"
}

#include "FFmpegAudio.h"
#include "FFmpegVedio.h"


ANativeWindow *window=0;
const char *filepath;
FFmpegVedio *Vedio;
FFmpegAudio *Audio;

//解码线程
pthread_t  p_tid;

int isPlay = 0;


void call_video_play(AVFrame *frame){
    if (!window) {
        return;
    }
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        return;
    }

    LOGE("绘制 宽%d,高%d",frame->width,frame->height);
    LOGE("绘制 宽%d,高%d  行字节 %d ",window_buffer.width,window_buffer.height, frame->linesize[0]);
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int dstStride = window_buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}

//解码函数call
void *process(void *args) {

    LOGE("ffmpeg init ");
//============ ffmpeg pcm ==================
//封装格式的上下文
    AVFormatContext		*pFormatCtx;
    int					i, audioindex;
    AVCodecContext		*pCodecCtxVedio, *pCodecCtxAudio;
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


    //注册解码器
    av_register_all();

    //初始化网络
    avformat_network_init();

    //查找封装格式的
    pFormatCtx = avformat_alloc_context();
    //文件路径

    LOGE("process     ========  文件路径%s", filepath);

        //打开文件
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        LOGE("%s","打开输入视频文件失败");
        return NULL;
    }
    LOGE("ffmpeg ====  open file ");

    //查找文件的信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        LOGE("%s","获取视频信息失败");
        return NULL;
    }
     LOGE("ffmpeg ====  find_stream_info ");
        //获取到音频的索引
        audioindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        //获取mp3 yuv解码器
        pCodecCtxAudio = pFormatCtx->streams[i]->codec;

        //查找音频,  视频的解码器
        pCodec = avcodec_find_decoder(pCodecCtxAudio->codec_id);

		pCodecCtxVedio = avcodec_alloc_context3(pCodec);
		
		avcodec_copy_context(pCodecCtxVedio, pCodecCtxAudio);
		
		if(avcodec_open2(pCodecCtxVedio, pCodec,NULL) < 0){
            LOGE("%s","解码器无法打开");
            continue;
        }
		
        //判断是否是频索引
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGE("  找到视频id %d", pFormatCtx->streams[i]->codec->codec_type);
            Vedio->setAvCodecContext(pCodecCtxVedio);
            Vedio->index = i; //id
            //PST
            Vedio->time_base = pFormatCtx->streams[i]->time_base;
            //设置绘制界面
            if (window) {
                ANativeWindow_setBuffersGeometry(window, Vedio->codec->width,
                                                 Vedio->codec->height,
                                                 WINDOW_FORMAT_RGBA_8888);
            }

        }

        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGE("  找到音频id %d", pFormatCtx->streams[i]->codec->codec_type);
            Audio->setAvCodecContext(pCodecCtxVedio);
            Audio->index = i;
            //PST
            Audio->time_base = pFormatCtx->streams[i]->time_base;

        }

    }
    LOGE("player 播放音视频bagin");

    //开启音频 视频播放 循环
    Vedio->setAudio(Audio);
   Vedio->play();
    Audio->play();
    isPlay = 1;

    //解码 packet
    //编码数据
    packet= (AVPacket *)av_malloc(sizeof(AVPacket));

    while (isPlay /*&& av_read_frame(pFormatCtx, packet) == 0*/) {
        //如果这个packet  流索引等于 视频流索引 添加到视频队列
        //        如果这个packet  流索引 等于 视频流索引 添加到视频队列
        ret = av_read_frame(pFormatCtx, packet);
        if (ret == 0) {
            if (Vedio && Vedio->isPlay && packet->stream_index == Vedio->index) {
                Vedio->put(packet);
            } else if (Audio && Audio->isPlay && packet->stream_index == Audio->index) {
                Audio->put(packet);
            }
            av_packet_unref(packet);
        } else if (ret == AVERROR_EOF) {
            //读完了
            //读取完毕 但是不一定播放完毕
            while (isPlay) {
                if (Vedio->queue.empty() && Audio->queue.empty()) {
                    break;
                }
                 LOGI("等待播放完成");
                av_usleep(10000);
            }
        }
        ///usleep( 26 * 1000);

    }
    //本地视频 有bug


    //视频解码完成
    isPlay = 0;

    if (Vedio && Vedio->isPlay) {
        Vedio->stop();
    }
    if (Audio && Audio->isPlay) {
        Audio->stop();
    }

    av_free_packet(packet);
    avformat_free_context(pFormatCtx);
    pthread_exit(NULL);
    return NULL;
}

/*
 * Class:     activity_songli_com_songlihlcplayer_SongliPlayer
 * Method:    player
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_activity_songli_com_songlihlcplayer_SongliPlayer_player
        (JNIEnv * env, jobject obj, jstring input_str)
{
    filepath = env->GetStringUTFChars(input_str, 0);

    LOGE("实例化对象filepath %s", filepath);
    //实例化对象
    Vedio = new FFmpegVedio;
    Audio = new FFmpegAudio;
    LOGE("对象new end");
    //设置视频
    Vedio->setPlayCall(call_video_play);
    pthread_create(&p_tid, NULL, process,NULL);
}


JNIEXPORT void JNICALL Java_activity_songli_com_songlihlcplayer_SongliPlayer_display
        (JNIEnv *env, jobject obj, jobject surface)
{
    //init

    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
    //设置宽高
    if (Vedio && Vedio->codec) {
        ANativeWindow_setBuffersGeometry(window, Vedio->codec->width,
                                         Vedio->codec->height,
                                         WINDOW_FORMAT_RGBA_8888);
    }
}
/*
 * Class:     activity_songli_com_songlihlcplayer_SongliPlayer
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_activity_songli_com_songlihlcplayer_SongliPlayer_release
        (JNIEnv *env , jobject obj)
{
    if (isPlay) {
        isPlay = 0;
        pthread_join(p_tid, 0);
    }
    if (Vedio) {
        if (Vedio->isPlay) {
            Vedio->stop();
        }
        delete (Vedio);
        Vedio = 0;
    }
    if (Audio) {
        if (Audio->isPlay) {
            Audio->stop();
        }
        delete (Audio);
        Audio = 0;

    }
}
