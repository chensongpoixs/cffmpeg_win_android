// Author  : songli on 2017/11/4 0004.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#ifndef SONGLIHLCPLAYER_FFMPEGVEDIO_H
#define SONGLIHLCPLAYER_FFMPEGVEDIO_H
#include <queue>
#include "FFmpegAudio.h"
extern  "C"
{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/time.h"
#include <pthread.h>
#include <unistd.h>
#include "LOG.h"
};



class FFmpegVedio {

public:
    FFmpegVedio();
    ~FFmpegVedio();

    int get(AVPacket *avPacket); //入队列

    int put(AVPacket *avPacket); //出队列

    void play(); //播放
    void setAvCodecContext(AVCodecContext *codecContext); //解码器


    void stop(); //停止



    void setPlayCall(void (*call)(AVFrame *frame));

    double synchronize(AVFrame *frame, double play);

    void setAudio(FFmpegAudio *audio);
public:
    int isPlay; //是否正在播放

    int index; //流索引

    std::queue<AVPacket*> queue; //视频队列



    AVRational time_base; //PST

    pthread_t  p_playid; //处理线程


    AVCodecContext *codec;


    //同步锁
    pthread_mutex_t mutex;
    //条件变量
    pthread_cond_t cond;




    //===========  ffmmepg Audio ==============
    FFmpegAudio* Audio;

    double  clock;
};
#endif //SONGLIHLCPLAYER_FFMPEGVEDIO_H
