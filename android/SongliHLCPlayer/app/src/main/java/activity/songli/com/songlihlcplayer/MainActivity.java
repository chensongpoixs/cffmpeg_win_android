package activity.songli.com.songlihlcplayer;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    SongliPlayer songliPlayer;

    private Spinner sp_video;
    private Button player;
    SurfaceView surfaceView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        sp_video = (Spinner) findViewById(R.id.sp_video);
        player = (Button) findViewById(R.id.player);
        surfaceView = (SurfaceView) findViewById(R.id.surface);
        songliPlayer = new SongliPlayer();
        //绘制
        songliPlayer.setSurfaceView(surfaceView);
        //多种格式的视频列表
        String[] videoArray = getResources().getStringArray(R.array.video_list);
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1,
                android.R.id.text1, videoArray);
        sp_video.setAdapter(adapter);

        player.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                //songliPlayer.playJava("rtmp://live.hkstv.hk.lxdns.com/live/hks");
                String video = sp_video.getSelectedItem().toString();
                String input = new File(Environment.getExternalStorageDirectory(), video).getAbsolutePath();
                songliPlayer.playJava(input);
                Log.d("songli", "player");
            }
        });
    }
    //public void player(View view) {
//        songliPlayer = new SongliPlayer();
//        //songliPlayer.player("rtmp://live.hkstv.hk.lxdns.com/live/hks");
//        String video = sp_video.getSelectedItem().toString();
//        String input = new File(Environment.getExternalStorageDirectory(), video).getAbsolutePath();
//        songliPlayer.player(input);
//        Log.d("songli", "player");
    //}
    public void stop(View view) {
        songliPlayer.release();
    }

}
