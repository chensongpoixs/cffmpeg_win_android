#include <jni.h>
#include <string>
#include <android/log.h>
extern "C" {
//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
//像素处理
#include "libswscale/swscale.h"
#include <android/native_window_jni.h>
#include <unistd.h>
}

#define LOGI(FORMAT,...) __android_log_print(ANDROID_LOG_INFO,"songli",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR,"songli",FORMAT,##__VA_ARGS__);


//JNIEXPORT void JNICALL
//Java_com_songli_view_VideoView_render(JNIEnv *env, jobject instance, jstring input_,
//                                             jobject surface) {
//    const char *input = env->GetStringUTFChars(input_, 0);
//
//    // TODO
//    av_register_all();
//
//    AVFormatContext *pFormatCtx = avformat_alloc_context();
//    //第四个参数是 可以传一个 字典   是一个入参出参对象
//    if (avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0) {
//        LOGE("%s","打开输入视频文件失败");
//    }
//    //3.获取视频信息
//    if(avformat_find_stream_info(pFormatCtx,NULL) < 0){
//        LOGE("%s","获取视频信息失败");
//        return;
//    }
//
//
//    int vidio_stream_idx=-1;
//    int i=0;
//    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
//        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//            LOGE("  找到视频id %d", pFormatCtx->streams[i]->codec->codec_type);
//            vidio_stream_idx=i;
//            break;
//        }
//    }
//
////    获取视频编解码器
//    AVCodecContext *pCodecCtx=pFormatCtx->streams[vidio_stream_idx]->codec;
//    LOGE("获取视频编码器上下文 %p  ",pCodecCtx);
////    加密的用不了
//    AVCodec *pCodex = avcodec_find_decoder(pCodecCtx->codec_id);
//    LOGE("获取视频编码 %p",pCodex);
////版本升级了
//    if (avcodec_open2(pCodecCtx, pCodex, NULL)<0) {
//
//
//    }
//    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
////    av_init_packet(packet);
////    像素数据
//    AVFrame *frame;
//    frame = av_frame_alloc();
////    RGB
//    AVFrame *rgb_frame = av_frame_alloc();
////    给缓冲区分配内存
//    //只有指定了AVFrame的像素格式、画面大小才能真正分配内存
//    //缓冲区分配内存
//    uint8_t   *out_buffer= (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height));
//    LOGE("宽  %d,  高  %d  ",pCodecCtx->width,pCodecCtx->height);
////设置yuvFrame的缓冲区，像素格式
//    int re= avpicture_fill((AVPicture *) rgb_frame, out_buffer, AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);
//    LOGE("申请内存%d   ",re);
//
////    输出需要改变
//    int length=0;
//    int got_frame;
////    输出文件
//    int frameCount=0;
//    SwsContext *swsContext = sws_getContext(pCodecCtx->width,pCodecCtx->height,pCodecCtx->pix_fmt,
//                                            pCodecCtx->width,pCodecCtx->height,AV_PIX_FMT_RGBA,SWS_BICUBIC,NULL,NULL,NULL
//    );
//    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
////    视频缓冲区
//    ANativeWindow_Buffer outBuffer;
////    ANativeWindow
//    while (av_read_frame(pFormatCtx, packet)>=0) {
////        AvFrame
//        if (packet->stream_index == vidio_stream_idx) {
//            length = avcodec_decode_video2(pCodecCtx, frame, &got_frame, packet);
//            LOGE(" 获得长度   %d ", length);
//
////非零   正在解码
//            if (got_frame) {
////            绘制之前   配置一些信息  比如宽高   格式
//                ANativeWindow_setBuffersGeometry(nativeWindow, pCodecCtx->width, pCodecCtx->height,
//                                                 WINDOW_FORMAT_RGBA_8888);
////            绘制
//                ANativeWindow_lock(nativeWindow, &outBuffer, NULL);
////     h 264   ----yuv          RGBA
//                LOGI("解码%d帧",frameCount++);
//                //转为指定的YUV420P
//                sws_scale(swsContext, (const uint8_t *const *) frame->data, frame->linesize, 0
//                        , pCodecCtx->height, rgb_frame->data,
//                          rgb_frame->linesize);
////rgb_frame是有画面数据
//                uint8_t *dst= (uint8_t *) outBuffer.bits;
////            拿到一行有多少个字节 RGBA
//                int destStride=outBuffer.stride*4;
////像素数据的首地址
//                uint8_t * src= (uint8_t *) rgb_frame->data[0];
////            实际内存一行数量
//                int srcStride = rgb_frame->linesize[0];
//                int i=0;
//                for (int i = 0; i < pCodecCtx->height; ++i) {
////                memcpy(void *dest, const void *src, size_t n)
//                    memcpy(dst + i * destStride,  src + i * srcStride, srcStride);
//                }
////
//                ANativeWindow_unlockAndPost(nativeWindow);
//                usleep(1000 * 16);
//            }
//        }
//        av_free_packet(packet);
//    }
//    ANativeWindow_release(nativeWindow);
//    av_frame_free(&frame);
//    avcodec_close(pCodecCtx);
//    avformat_free_context(pFormatCtx);
//    LOGE("%s", "so000000000000000000000gn");
//    env->ReleaseStringUTFChars(input_, input);
//}

extern  "C"
JNIEXPORT void JNICALL
Java_com_songli_view_VideoView_render(JNIEnv *env, jobject obj, jstring inputStr_, jobject surface)
{
    // string 转char*类型
    const char *inputStr = env->GetStringUTFChars(inputStr_, NULL);
	LOGE("%s", "ffmpeg");
    //================ ffmpeg ==============================
    //封装格式的上下文
    AVFormatContext     *pFormatCtx;
    int                 i,  videoindex;
    AVCodecContext      *pCodecCtx;
    AVCodec             *pCodec;
    AVFrame             *pFrame, *pFrameRGB;
    //缓冲区
    uint8_t             *out_buff;
    int                 ret;
    int                 got_pictrue, frameindex;
    AVPacket            *packet;
    struct SwsContext   *swsContext;

    //==================  NationWindow  ==========================
    ANativeWindow       * nativeWindow;

    //视频native缓冲区
    ANativeWindow_Buffer out_win_native_buff;

    int                 desStride, srcStride;
    //像素首地址
    uint8_t             *src;



    //注册所有的解码器
    av_register_all();

    // 初始化封装格式format
    avformat_network_init();

    //得到封装格式
    pFormatCtx = avformat_alloc_context();

    //打开文件
    //avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);

    if (avformat_open_input(&pFormatCtx, inputStr, NULL, NULL) != 0)
    {
        LOGE("%s", "Couldn't open stream ");
        return;
    }


    //查找封装格式的信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        LOGE("%s", "Couldn't find stream information.");
        return;
    }


    //查找视频流索引
    videoindex = -1;
    for (i = 0; pFormatCtx->nb_streams; i++)
    {
        //判断是否视频流的索引
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }

    if (videoindex == -1)
    {
        LOGE("%s", "Couldn't video stream .");
        return;
    }


    //拿到视频解码器
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;

    //获取264的解码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if (!pCodec)
    {
        LOGE("%s", "Could not find codec.\n");
        return;
    }

    //打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        LOGE("%s", "Couldn't not open codec.");
        return;
    }

    //开辟内存包视频帧的内存
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    //要转成RGB格式的
    out_buff = (uint8_t*)av_malloc(
            avpicture_get_size(AV_PIX_FMT_RGBA,
                               pCodecCtx->width, pCodecCtx->height));

    //使用rgbfill缓冲区

     int avpicture_fill(AVPicture *picture, const uint8_t *ptr,
                   enum AVPixelFormat pix_fmt, int width, int height);

    ret = avpicture_fill((AVPicture *)pFrameRGB,
                         out_buff, AV_PIX_FMT_RGBA,
                         pCodecCtx->width, pCodecCtx->height
    );

    //申请packet内存
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    if (ret < 0)
    {
        LOGE("%s", "Couldn't malloc 内存");
        return;
    }

    //转换格式

    struct SwsContext *sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat,
                                  int dstW, int dstH, enum AVPixelFormat dstFormat,
                                  int flags, SwsFilter *srcFilter,
                                  SwsFilter *dstFilter, const double *param);

    swsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                                pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGBA,
                                SWS_BICUBIC, NULL, NULL, NULL );


    //windows
    nativeWindow = ANativeWindow_fromSurface(env, surface);


    frameindex = 0;
    //读取视频一帧的读取
    while(av_read_frame(pFormatCtx, packet) >= 0)
    {
        LOGE("%s", "777777777777777777");
        //判断是否是视频流的索引
        if (packet->stream_index == videoindex)
        {

            //获取他的长度
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_pictrue, packet);
            if (ret < 0)
            {
                LOGE("%", "Decode Error.");
                return;
            }

            //解码开始和绘制的步骤
            if (got_pictrue)
            {
                //绘制前设置屏幕的宽度和高度


                //int32_t ANativeWindow_setBuffersGeometry(ANativeWindow* window,
                  //                                       int32_t width, int32_t height, int32_t format);

                ANativeWindow_setBuffersGeometry(nativeWindow, pCodecCtx->width, pCodecCtx->height,
                                                 WINDOW_FORMAT_RGBA_8888);
                //绘制
                ANativeWindow_lock(nativeWindow, &out_win_native_buff, NULL);

                //=====================h264 --->  RGB====================================
                //h264 --------------------------> RGB

                  int sws_scale(struct SwsContext *c, const uint8_t *const srcSlice[],
              const int srcStride[], int srcSliceY,int srcSliceH,
              uint8_t *const dst[], const int dstStride[]);

                sws_scale(swsContext, (const uint8_t* const *)pFrame->data, pFrame->linesize, 0,
                pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                // pFrameRGB 数据
                uint8_t  *dst = (uint8_t *)out_win_native_buff.bits;
                //一行的多少字节 RGB
                desStride = out_win_native_buff.stride * 4;

                //像素数据的首地址
                src = (uint8_t *)pFrameRGB->data[0];
                //实际内存一行数量
                srcStride = pFrameRGB->linesize[0];

                for (i = 0; i < pCodecCtx->height; ++i)
                {
                    memcpy(dst + i * desStride, src + i * srcStride, srcStride);
                }
                LOGE("%s %d", "Decode stream frame index: "  + frameindex);
                frameindex++;

                ANativeWindow_unlockAndPost(nativeWindow);
                //暂停16秒
                usleep(1000 * 16);
            }
        }
        //av_free_packet(packet);
    }




    //释放nativewindow
    ANativeWindow_release(nativeWindow);
    //释放frame内存
    av_frame_free(&pFrame);
    av_frame_free(&pFrameRGB);
    avformat_free_context(pFormatCtx);
//释放char*内存
    env->ReleaseStringUTFChars(inputStr_, inputStr);
}