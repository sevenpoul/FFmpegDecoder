package com.ffmpeg;

import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.znv.decoder.NativeLib;

import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends Activity {

    private SurfaceView mTestSurface0, mTestSurface1;
    private Surface mSurface0, mSurface1;
    private NativeLib[] mNative = new NativeLib[2];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTestSurface0 = (SurfaceView) findViewById(R.id.surface_0);
        mTestSurface1 = (SurfaceView) findViewById(R.id.surface_1);

        mTestSurface0.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                Log.d("yhw", "surfaceCreated 0");
                mSurface0 = surfaceHolder.getSurface();
                mNative[0] = new NativeLib(mSurface0, 640, 480);
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                Log.d("yhw", "surfaceDestroyed 0");
                mNative[0].close();
            }
        });

        mTestSurface1.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                mSurface1 = surfaceHolder.getSurface();
                mNative[1] = new NativeLib(mSurface1, 1280, 960);
                Log.d("yhw", "surfaceCreated 1");
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                Log.d("yhw", "surfaceDestroyed 1");
                mNative[1].close();
            }
        });

        findViewById(R.id.button_0).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Thread thread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        test_decode(mNative[0], R.raw.min_cif);
                    }
                });
                thread.start();
            }
        });

        findViewById(R.id.button_1).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Thread thread = new Thread(new Runnable() {
                    @Override
                    public void run() {
                        test_decode(mNative[1], R.raw.min_1080p);
                    }
                });
                thread.start();
            }
        });
    }

    private void test_decode(NativeLib nativeLib, int resourceId){
        InputStream inputStream = getResources().openRawResource(resourceId);
        byte[] buffer = new byte[1024*8];
        int len;
        try {
            while ((len = inputStream.read(buffer)) != -1) {
                nativeLib.decodeStream(buffer, len);
            }
        }catch (Exception e) {
            e.printStackTrace();
        }
    }
}
