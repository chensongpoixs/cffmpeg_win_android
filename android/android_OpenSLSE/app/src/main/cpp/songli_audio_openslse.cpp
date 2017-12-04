#include <jni.h>
#include <string>

extern "C"
{
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"
#include <android/native_window_jni.h>
#include <unistd.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES.h>

#include "com_openslse_Activity_SongliPlayer.h"
}
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"songli",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"songli",FORMAT,##__VA_ARGS__);

#include "FfmpegMusic.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_openslse_Activity_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

//引擎接口
SLObjectItf engineObject;

SLEngineItf engineEngine;

//混音器
SLObjectItf  outputMixObject;


//环境混响
SLEnvironmentalReverbItf  outputMixEnvironmentanlReverbItf;


//设置环境 默认的环境
SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;


//播放
SLObjectItf  bgPlayerObject;

//播放接口
SLPlayItf  bgPlayerplay;

//缓冲区
SLAndroidSimpleBufferQueueItf  bgPlayerQueue;


//音量对象
SLVolumeItf  bqPalyerVolume;

size_t bufferSize = 0;

void *buffer;



//回调函数指针   只要喇叭一读取数据就 添加pcm队列中
void bqPlayerCallBack(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    bufferSize = 0;
    //取到音频数据
    getPcm(&buffer, &bufferSize);

    if (NULL != buffer && 0 != bufferSize)
    {
        //播放的地方
        SLresult  sLresult = (*bgPlayerQueue)->Enqueue(bgPlayerQueue, buffer, bufferSize);
        LOGE("正在播放%d", sLresult);
    }
}


extern "C"
JNIEXPORT void JNICALL Java_com_openslse_Activity_SongliPlayer_player
        (JNIEnv *env, jobject obj)
{

    LOGE("OpenSLES");
    SLresult sLresult;

    //初始化一个引擎
    slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);

    (*engineObject)->Realize(engineObject,
                             SL_BOOLEAN_FALSE);

    //1, 获取到引擎接口  获取引擎接口对象   参数二：音频的id
    (*engineObject)->GetInterface(engineObject,
                                  SL_IID_ENGINE,
                                  &engineEngine);

    LOGE("引擎地址%p ", engineEngine);

    //2, 创建输出混音器
    (*engineEngine)->CreateOutputMix(engineEngine,
                                     &outputMixObject,
                                     0, 0, 0);

    //同步的操作
    (*outputMixObject)->Realize(outputMixObject,
                                SL_BOOLEAN_FALSE);


    //3, 设置环境混响
    sLresult = (*outputMixObject)->GetInterface(outputMixObject,
                                     SL_IID_ENVIRONMENTALREVERB,  //设置环境混响
                                     &outputMixEnvironmentanlReverbItf);

    LOGE("slresult %p", sLresult);
    if (sLresult == SL_RESULT_SUCCESS)   //配置问题
    {
        (*outputMixEnvironmentanlReverbItf)->SetEnvironmentalReverbProperties(outputMixEnvironmentanlReverbItf,
                                                                              &settings);
    }


    //===========init ffmepg ======================

    int rate;
    int channers;
    createFFmepg(&rate, &channers);

    LOGE("初始化ffmpeg");

    //缓冲区队列  设置文件是文件函数网络的数据
    SLDataLocator_AndroidBufferQueue android_qunue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    //参数二：通道数， 参数三：采样， 参数四：采样位数， 参数⑤：
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM,
                            (SLuint32)channers,
                            SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT| SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    //编码格式
    SLDataSource slDataSource = {&android_qunue, &pcm };


    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX,
                                         outputMixObject};
    //混音器关联起来
    SLDataSink audioSnk = {&outputMix, NULL};
    //设置声音
    SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,
                            SL_IID_EFFECTSEND,
                            SL_IID_VOLUME};

    SLboolean req[3] = {SL_BOOLEAN_FALSE };

    LOGE("引擎%p", engineEngine);

    //4, 创建一个播放
    /**参数二: 引擎的对象
     * 参数三： 编码格式
     *
     */
    sLresult = (*engineEngine)->CreateAudioPlayer(engineEngine,
                                       &bgPlayerObject,
                                       &slDataSource,
                                       &audioSnk,
                                       3,
                                       ids,
                                       req);

    LOGE(" sLresult = %d , bgPlayerObject%p slDataSoucre%p", sLresult, bgPlayerObject, slDataSource);
    (*bgPlayerObject)->Realize(bgPlayerObject, SL_BOOLEAN_FALSE);


    //创建一个播放接口
    (*bgPlayerObject)->GetInterface(bgPlayerObject,
                                  SL_IID_PLAY, &bgPlayerplay);

    //注册缓冲区
    (*bgPlayerObject)->GetInterface(bgPlayerObject,
                                    SL_IID_BUFFERQUEUE,
                                    &bgPlayerQueue);

    //设置回调接口 getpcm
    (*bgPlayerQueue)->RegisterCallback(bgPlayerQueue, bqPlayerCallBack, NULL);

    //音量
    (*bgPlayerObject)->GetInterface(bgPlayerObject,
                                    SL_IID_VOLUME,
                                    &bqPalyerVolume);

    //设置播放状态
    (*bgPlayerplay)->SetPlayState(bgPlayerplay, SL_PLAYSTATE_PLAYING);

    //播放第一帧
    bqPlayerCallBack(bgPlayerQueue, NULL);


}


extern "C"
JNIEXPORT void JNICALL Java_com_openslse_Activity_SongliPlayer_stop
        (JNIEnv *env, jobject obj)
{
    //清空内存
   if (bgPlayerObject != NULL)
   {
       //销毁内存
       (*bgPlayerObject)->Destroy(bgPlayerObject);
        bgPlayerObject = NULL;
       bgPlayerplay = NULL;
        bgPlayerQueue = NULL;
       bqPalyerVolume = NULL;
   }

    //混音器释放
    if (outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentanlReverbItf = NULL;
    }

    //引擎释放
    if (engineObject != NULL)
    {
        (*engineObject)->Destroy(engineObject);

        engineObject = NULL;
        engineEngine = NULL;
    }

    //ffmepg free
    realseFFmpeg();
}

