// Author  : songli on 2017/11/4 0004.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#include "FFmpegAudio.h"

//==========================


int createFFmepg(FFmpegAudio *Audio)
{
   //信息
    //av_dump_format(Audio-, 0, input, 0);



    //mp3 -> pcm 抽样的数据    转换器
    Audio->swrContext = swr_alloc();


    int length = 0;
    int got_frame;
//    44100*2
    Audio->out_buff = (uint8_t *) av_malloc(44100 * 2);
    uint64_t  out_ch_layout = AV_CH_LAYOUT_STEREO;
//    输出采样位数  16位
    enum AVSampleFormat out_formart=AV_SAMPLE_FMT_S16;

    //输出的采样率必须与输入相同
    int out_sample_rate = Audio->codec->sample_rate;
    //转换格式
    swr_alloc_set_opts(Audio->swrContext,
                       out_ch_layout,
                       out_formart,
                       out_sample_rate,
                       Audio->codec->channel_layout,
                       Audio->codec->sample_fmt,
                       Audio->codec->sample_rate, 0, NULL
    );

    //初始化采样
    swr_init(Audio->swrContext);





    //Audio->out_buff = (uint8_t *) av_malloc(44100 * 2);
//    获取通道数  2
    Audio->out_channer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    LOGE("------>通道数%d  ", Audio->out_channer_nb);

    LOGE("ffmpeg初始化完成");
    return 0;
}

/**
 * 得到解码后的数据
 * @param Audio
 */
int getPcm(FFmpegAudio *Audio) {
    int frameCount = 0;
    int got_frame;
    AVFrame *frame = av_frame_alloc();
    int out_buffer_size;
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //获取到通道数z
    //int out_chananer_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    //读取一帧的数据
    while (Audio->isPlay) {
        out_buffer_size = 0;

        //取数据packet
        Audio->get(packet);  //入参出函数

        //================  PST  ===============
        if (packet->pts != AV_NOPTS_VALUE) {
            //   一份子 和 时间
            Audio->clock = packet->pts * av_q2d(Audio->time_base);
        }

        //解码器
        int ret = avcodec_decode_audio4(Audio->codec,
                              frame,
                              &got_frame,
                              packet);
//        if (ret < 0) {
//            LOGE("解码Didn't audio Codec Error.");
//        }

        if (got_frame) {
            //==============================================================
            swr_convert(Audio->swrContext, &Audio->out_buff, 44100 * 2,
                        (const uint8_t **) frame->data,
                        frame->nb_samples);
            //缓冲区的大小
            out_buffer_size = av_samples_get_buffer_size(NULL,
                                                         Audio->out_channer_nb,
                                                         frame->nb_samples,
                                                         AV_SAMPLE_FMT_S16, 1);
            //===============================================================
           // LOGE("Decodec stream freameindex:");
           // frameindex++;
            LOGE("out_buffer_size %d", out_buffer_size);

            break;
        }
    }

    av_free(packet);
    av_frame_free(&frame);
    LOGE("out_buffer_size %d", out_buffer_size);
    return out_buffer_size;
}



////释放内存
//void realseFFmpeg()
//{
//
//    av_free_packet(packet);
//    //缓冲区
//    av_free(out_buff);
//    swr_free(&swrContext);
//    av_frame_free(&pFrame);
//    avcodec_close(pCodecCtx);
//    //关闭文件
//    avformat_close_input(&pFormatCtx);
//
//}






//=========== ffmpeg end ==============



void *play_audio(void *arg) {
    LOGE("开启音频线程");

    FFmpegAudio *Audio  = (FFmpegAudio *)arg;

        //不断的播放
    Audio->createPlayer();
    LOGE("createplayer");

    //退出
    pthread_exit(NULL);
//    AVFrame *frame = av_frame_alloc();
//
//    AVPacket *packet = (AVPacket*)av_malloc(sizeof(AVPacket));
//
//    //mp3 转换 pcm
//    SwrContext *swrContext = swr_alloc();
//
//    int length = 0;
//
//    //输出
//
//    swr_alloc_set_opts(swrContext,
//                       AV_CH_LAYOUT_STEREO,
//                       AV_SAMPLE_FMT_S16,
//                       Audio->codec->sample_rate,
//                       Audio->codec->channel_layout,
//                       Audio->codec->sample_fmt,
//                       Audio->codec->sample_rate, 0, 0);
//
//    swr_init(swrContext);
//
//    uint8_t  *out_buffer = (uint8_t *)av_malloc(44100 * 2 * 2);
//
//    //获取通道数
//    int channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
//
//    int got_frame;
//    //死循环
//    while (Audio->isPlay) {
//        //取出一个音频packet
//        Audio->get(packet);
//        avcodec_decode_audio4(Audio->codec, frame, &got_frame, packet);
//
//
//
//        if (got_frame) {
//            LOGE("解码音频帧");
//            swr_convert(swrContext, &out_buffer,
//                        44100 * 2 *2,
//                        (const uint8_t **) frame->data,
//                        frame->nb_samples);
//
//            int out_buffer_size = av_samples_get_buffer_size(NULL,
//                                                             channels,
//                                                             frame->nb_samples,
//                                                             AV_SAMPLE_FMT_S16,
//                                                             1);
//        }
//    }
    return NULL;
}


//第一次主动调用在调用线程
//之后在新线程中回调
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    FFmpegAudio  *Audio = (FFmpegAudio *) context;
    LOGE("player Callback Open SE SL");
    //取数据的过程
    int datalen = getPcm(Audio);
    LOGE("data %d", datalen);
    if (datalen > 0) {
        double time = datalen / ((double) 44100 * 2 * 2);
        LOGE("数据长度%d  分母%d  值%f 通道数%d",datalen, 44100 *2 * 2, time,Audio->out_channer_nb);
        Audio->clock = Audio->clock + time;
        LOGE("当前一帧声音时间%f   播放时间%f", time, Audio->clock);
        (*bq)->Enqueue(bq, Audio->out_buff, datalen);
        LOGE("播放 %d ",Audio->queue.size());
    } else
        LOGE("解码错误");
}

//======================================= call end=====================



FFmpegAudio::FFmpegAudio() {
    this->clock = 0;
    //初始化锁
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL); //初始化条件变量
}

//生产者
int FFmpegAudio::put(AVPacket *avPacket) {
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    //克隆  ===   拷贝意思
    if (av_packet_ref(packet, avPacket)) {
        LOGE("packet 克隆失败！");
        return 0;
    }
    LOGE("压入一帧音频数据   %d", queue.size());

    //加锁
    pthread_mutex_lock(&mutex);
    queue.push(packet);
    LOGE("压入一帧音频数据  队列%d ",queue.size());

    //解锁
    pthread_mutex_unlock(&mutex);
    //通知消费者 ， 给消费者解锁
    pthread_cond_signal(&cond);
    return 1;
}

//消费者
int FFmpegAudio::get(AVPacket *avPacket) {

    pthread_mutex_lock(&mutex);

    while (isPlay) {
        if (!queue.empty()) {
            //从队列取出一个packet
            if (av_packet_ref(avPacket, queue.front())) {
                break;
            }

            //取成功了 弹出队列 销毁packet
            AVPacket *pkt = queue.front();
            queue.pop();
            LOGE("取出一帧音频%d", queue.size());
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

void FFmpegAudio::play() {
    isPlay  = 1;
//开启解码
    pthread_create(&p_playid, NULL, play_audio, this);
}


int FFmpegAudio::createPlayer() {

    LOGE("OpenSE SL")
    SLresult result;
    // 创建引擎engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 实现引擎engineObject
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 获取引擎接口engineEngine
    result = (*engineObject)->GetInterface(engineObject,
                                           SL_IID_ENGINE,
                                           &engineEngine);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    // 创建混音器outputMixObject
    result = (*engineEngine)->CreateOutputMix(engineEngine,
                                              &outputMixObject,
                                              0,
                                              0,
                                              0);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }



    // 实现混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject,
                                         SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return 0;
    }
    result = (*outputMixObject)->GetInterface(outputMixObject,
                                              SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);

    const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    if (SL_RESULT_SUCCESS == result) {
        (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &settings);
    }


    //======================
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                            2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,
                            2,
                            SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
//   新建一个数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};
//    设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,
                                         outputMixObject};

    SLDataSink audioSnk = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,
                                  SL_IID_EFFECTSEND,
            /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,
            /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    //先讲这个
    (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &slDataSource,
                                       &audioSnk, 2,
                                       ids, req);
    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

//    得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);

//    注册回调缓冲区 //获取缓冲队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                    &bqPlayerBufferQueue);
    //缓冲接口回调
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);
//    获取音量接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);

//    获取播放状态接口
    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    LOGE("SE SL");
    bqPlayerCallback(bqPlayerBufferQueue, this);
    return 1;
}

void FFmpegAudio::stop() {
    LOGE("声音暂停");
    //因为可能卡在 deQueue
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(p_playid, 0);
    if (bqPlayerPlay) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        bqPlayerPlay = 0;
    }
    if (bqPlayerObject) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;

        bqPlayerBufferQueue = 0;
        bqPlayerVolume = 0;
    }

    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }

    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineEngine = 0;
    }
    if (swrContext)
        swr_free(&swrContext);
    if (this->codec) {
        if (avcodec_is_open(this->codec))
            avcodec_close(this->codec);
        avcodec_free_context(&this->codec);
        this->codec = 0;
    }
    LOGE("AUDIO clear");
}

//解码器
void FFmpegAudio::setAvCodecContext(AVCodecContext *codecContext) {
    this->codec = codecContext; //解码器
    //初始化ffmpeg
    createFFmepg(this);
}




FFmpegAudio::~FFmpegAudio() {

    if (out_buff) {
        free(out_buff);
    }
    for (int i = 0; i < queue.size(); ++i) {
        AVPacket *pkt = queue.front();
        queue.pop();
        LOGE("销毁音频帧%d",queue.size());
        av_free(pkt);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}




