package com.openslse.Activity;

/**
 * Author  : songli on 2017/10/24 0024.
 * CSDN   : http://blog.csdn.net/poisx
 * Github : https://github.com/chensongpoixs
 */

public class SongliPlayer {


    static {
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("songli_audio_openslse");
    }


    public native void player();
    public native void stop();

}
