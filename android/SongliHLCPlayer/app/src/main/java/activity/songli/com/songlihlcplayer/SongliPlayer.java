package activity.songli.com.songlihlcplayer;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Author  : songli on 2017/11/4 0004.
 * CSDN   : http://blog.csdn.net/poisx
 * Github : https://github.com/chensongpoixs
 */

public class SongliPlayer  implements SurfaceHolder.Callback{

    static {
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avutil-54");
        System.loadLibrary("postproc-53");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("SongliHLCPlayer");
    }
    private SurfaceView surfaceView;

    public   void playJava(String path) {
        Log.d("songli", "player Java");
        if (surfaceView == null) {
            Log.d("songli", "surfaceView NULL");
            return;
        }
        player(path);
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        this.surfaceView = surfaceView;
        display(surfaceView.getHolder().getSurface());
        surfaceView.getHolder().addCallback(this);

    }

    public native void player(String filepath);
    public native void display(Surface surface);

    public native void release();

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        display(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }
}
