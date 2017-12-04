// Author  : songli on 2017/11/4 0004.
//  CSDN   : http://blog.csdn.net/poisx
//  Github : https://github.com/chensongpoixs
//

#ifndef SONGLIHLCPLAYER_LOG_H
#define SONGLIHLCPLAYER_LOG_H

#include <android/log.h>
#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"songli",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"songli",FORMAT,##__VA_ARGS__);

#endif //SONGLIHLCPLAYER_LOG_H
