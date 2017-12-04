// Author  : songli on 2017/10/24 0024.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#ifndef ANDROID_OPENSLSE_FFMPEGMUSIC_H
#define ANDROID_OPENSLSE_FFMPEGMUSIC_H
extern  "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include <android/native_window_jni.h>
#include <unistd.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <android/log.h>

};
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"songli",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"songli",FORMAT,##__VA_ARGS__);



//初始化操作ffmpeg
//openslse 调用    参数: 通道数和声道
int createFFmepg(int *rate, int *channel);


//读取数据packet      参数一:pcm 数据    ,参数二:数组的多大
void getPcm(void **pcm, size_t *pcm_size);

void realseFFmpeg();

#endif //ANDROID_OPENSLSE_FFMPEGMUSIC_H
