// Author  : songli on 2017/11/4 0004.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#include "FFmpegVedio.h"


//call back
static void (*video_call)(AVFrame *frame);

void *play_Vedio(void *arg) {
    FFmpegVedio *Vedio = (FFmpegVedio *) arg;

    //像素数据
    AVFrame *frame = av_frame_alloc();
    //rgb数据
    AVFrame *rgb_frame = av_frame_alloc();

    //缓冲区
    uint8_t  *out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA,
                                                                    Vedio->codec->width,
                                                                    Vedio->codec->height));
    //设置yuvfrmae的缓冲区 ， 像素格式
    int re = avpicture_fill((AVPicture *) rgb_frame,
                   out_buffer,
                   AV_PIX_FMT_RGBA,
                   Vedio->codec->width,
                   Vedio->codec->height);
    LOGE("申请内存%d   ",re);

    //native绘制操作
    struct SwsContext *sws_ctx = sws_getContext(Vedio->codec->width,
                                                Vedio->codec->height,
                                                Vedio->codec->pix_fmt,
                                                Vedio->codec->width,
                                                Vedio->codec->height,
                                                AV_PIX_FMT_RGBA,
                                                SWS_BICUBIC,
                                                NULL, NULL, NULL);

    int len, got_frame, framecount = 0;
    LOGE("宽 %d, 高 %d", Vedio->codec->width, Vedio->codec->height);

    AVPacket *packet = (AVPacket*)av_malloc(sizeof(AVPacket));
    //6.一阵一阵读取压缩的视频数据AVPacket
    double  last_play  //上一帧的播放时间
    ,play             //当前帧的播放时间
    , last_delay    // 上一次播放视频的两帧视频间隔时间
    ,delay         //两帧视频间隔时间
    ,audio_clock //音频轨道 实际播放时间
    ,diff   //音频帧与视频帧相差时间
    ,sync_threshold
    ,start_time  //从第一帧开始的绝对时间
    ,pts
    ,actual_delay//真正需要延迟时间
    ;//两帧间隔合理间隔时间
    start_time = av_gettime() / 1000000.0;
    //一帧读取数据
    while (Vedio->isPlay) {
        LOGE("解码视频 一帧 %d ", Vedio->queue.size());
        //消费者取一帧数据
        Vedio->get(packet);
        len = avcodec_decode_video2(Vedio->codec, frame, &got_frame, packet);

        if (!got_frame) {
            LOGE("视频解码器失败");
            continue;
        }
        //转码rgb
        int codec = sws_scale(sws_ctx,
                  (const uint8_t *const *) frame->data,
                  frame->linesize, 0,
                  Vedio->codec->height,
                  rgb_frame->data,
                  rgb_frame->linesize);
        LOGE("codec = %d, len = %d ", codec, len );
//        LOGE("宽%d  高%d  行字节 %d  状态%d ====%d",
//             frame->width,
//             frame->height,
//             rgb_frame->linesize[0],
//             got_frame,
//             vedio->codec->height);
        //主动绘制 时间

        //===============    时间同步问题 ============================
        if ((pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE) {
            pts = 0;
        }
        play = pts * av_q2d(Vedio->time_base);
//        纠正时间
        play = Vedio->synchronize(frame, play);
        delay = play - last_play;
        if (delay <= 0 || delay > 1) {
            delay = last_delay;
        }
        audio_clock = Vedio->Audio->clock;
        last_delay = delay;
        last_play = play;
//音频与视频的时间差
        diff = Vedio->clock - audio_clock;
//        在合理范围外  才会延迟  加快
        sync_threshold = (delay > 0.01 ? 0.01 : delay);

        if (fabs(diff) < 10) {
            if (diff <= -sync_threshold) {
                delay = 0;
            } else if (diff >=sync_threshold) {
                delay = 2 * delay;
            }
        }
        start_time += delay;
        actual_delay = start_time-av_gettime() / 1000000.0;
        if (actual_delay < 0.01) {
            actual_delay = 0.01;
        }
        av_usleep(actual_delay * 1000000.0 + 6000);

        video_call(rgb_frame);
        //usleep(16 * 1000);

    }
    LOGE("free packet");
    av_free(packet);
    LOGE("free packet ok");
    LOGE("free packet");
    av_frame_free(&frame);
    av_frame_free(&rgb_frame);
    sws_freeContext(sws_ctx);
    size_t size = Vedio->queue.size();
    for (int i = 0; i < size; ++i) {
        AVPacket *pkt = Vedio->queue.front();
        av_free(pkt);
        Vedio->queue.pop();
    }
    LOGE("VIDEO EXIT");
    pthread_exit(0);
    return NULL;
}

FFmpegVedio::FFmpegVedio() {

    pthread_mutex_init(&mutex, NULL); //初始化锁
    pthread_cond_init(&cond, NULL); //初始化条件变量
}


//消费者
int FFmpegVedio::get(AVPacket *avPacket) {
    pthread_mutex_lock(&mutex);

    LOGE("消费者");
    while (isPlay) {
        if (!queue.empty()) {
            //从队列取出一个packet
            if (av_packet_ref(avPacket, queue.front())) {
                break;
            }

            //取成功了 弹出队列 销毁packet
            AVPacket *pkt = queue.front();
            queue.pop();
            LOGE("取出视频一帧：%d", queue.size());
            //释放
            av_free(pkt);
            break;
        } else {
            //如果队列没有数据一直等待
            pthread_cond_wait(&cond, &mutex);
        }
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}


//生产者
int FFmpegVedio::put(AVPacket *avPacket) {

    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //克隆  ===   拷贝意思
    if (av_copy_packet(packet, avPacket)) {
        LOGE("packet 克隆失败！");
        return 0;
    }
    LOGE("压入一帧视频数据");

    //加锁
    pthread_mutex_lock(&mutex);
    queue.push(packet);

    //通知消费者 ， 给消费者解锁
    pthread_cond_signal(&cond);

    //解锁
    pthread_mutex_unlock(&mutex);

    return 1;
}

void FFmpegVedio::play() {

    isPlay = 1;
    //开启解码
    pthread_create(&p_playid, NULL, play_Vedio, this);
}

void FFmpegVedio::stop() {
    LOGE("VIDEO stop");

    pthread_mutex_lock(&mutex);
    isPlay = 0;
    //因为可能卡在 deQueue
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(p_playid, 0);
    LOGE("VIDEO join pass");
    if (this->codec) {
        if (avcodec_is_open(this->codec))
            avcodec_close(this->codec);
        avcodec_free_context(&this->codec);
        this->codec = 0;
    }
    LOGE("VIDEO close");
}

FFmpegVedio::~FFmpegVedio() {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}

void FFmpegVedio::setAvCodecContext(AVCodecContext *codecContext) {
    this->codec = codecContext;
}

void FFmpegVedio::setPlayCall(void (*call)(AVFrame *)) {
    video_call = call;
}

double FFmpegVedio::synchronize(AVFrame *frame, double play) {
    //clock是当前播放的时间位置
    if (play != 0)
        clock=play;
    else //pst为0 则先把pts设为上一帧时间
        play = clock;
    //可能有pts为0 则主动增加clock
    //frame->repeat_pict = 当解码时，这张图片需要要延迟多少
    //需要求出扩展延时：
    //extra_delay = repeat_pict / (2*fps) 显示这样图片需要延迟这么久来显示
    double repeat_pict = frame->repeat_pict;
    //使用AvCodecContext的而不是stream的
    double frame_delay = av_q2d(codec->time_base);
    //如果time_base是1,25 把1s分成25份，则fps为25
    //fps = 1/(1/25)
    double fps = 1 / frame_delay;
    //pts 加上 这个延迟 是显示时间
    double extra_delay = repeat_pict / (2 * fps);
    double delay = extra_delay + frame_delay;
//    LOGI("extra_delay:%f",extra_delay);
    clock += delay;
    return play;
}

void FFmpegVedio::setAudio(FFmpegAudio *Audio) {
    this->Audio = Audio;
}
