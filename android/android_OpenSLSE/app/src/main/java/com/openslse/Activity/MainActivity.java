package com.openslse.Activity;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private static String TAG = "songli";
    private Button btnplayer;
    private Button btnstop;
    private SongliPlayer songliPlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        songliPlayer = new SongliPlayer();
        btnplayer = (Button) findViewById(R.id.player);
        btnstop = (Button) findViewById(R.id.stop);
        btnplayer.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                play();
            }
        });

        btnstop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                stop();
            }
        });

    }

    private void stop() {
        songliPlayer.stop();
    }


    public  void play()
    {

        songliPlayer.player();
        Log.i(TAG, "onClick");
    }


}
