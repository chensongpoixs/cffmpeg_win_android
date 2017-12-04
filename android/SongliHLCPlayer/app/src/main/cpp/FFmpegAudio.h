// Author  : songli on 2017/11/4 0004.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#ifndef SONGLIHLCPLAYER_FFMPEGAUDIO_H
#define SONGLIHLCPLAYER_FFMPEGAUDIO_H

#include <queue>
extern  "C"
{
#include <stdlib.h>
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
#include <pthread.h>
#include "LOG.h"
#include <android/native_window_jni.h>
#include <unistd.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>
};



class FFmpegAudio {
public:
    FFmpegAudio();
    ~FFmpegAudio();

    int get(AVPacket *avPacket); //入队列

    int put(AVPacket *avPacket); //出队列

    void play(); //播放


    void stop(); //停止

    int createPlayer();

    void setAvCodecContext(AVCodecContext *codecContext); //解码器


public:
    int isPlay; //是否正在播放

    int index; //流索引

    std::queue<AVPacket*> queue; //音频队列

    pthread_t  p_playid; //处理线程


    AVCodecContext *codec;
    //同步锁
    pthread_mutex_t mutex;
    //条件变量
    pthread_cond_t cond;


    SwrContext *swrContext;
    uint8_t * out_buff;

    //通道数
    int out_channer_nb;
//    相对于第一帧时间
    double clock;

    AVRational time_base; //PST


    //========== opense sl ================



    SLObjectItf engineObject;
    SLEngineItf engineEngine;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    SLObjectItf outputMixObject;
    SLObjectItf bqPlayerObject;
    SLEffectSendItf bqPlayerEffectSend;
    SLVolumeItf bqPlayerVolume;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
};
#endif //SONGLIHLCPLAYER_FFMPEGAUDIO_H
