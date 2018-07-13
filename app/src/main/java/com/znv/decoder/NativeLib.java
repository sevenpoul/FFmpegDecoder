package com.znv.decoder;

import android.view.Surface;

/**
 * Created by lishanshan on 2018/5/31.
 */

public class NativeLib {

    private Surface mSurface;
    private long handle;

    public NativeLib(Surface surface, int width, int height) {
        this.mSurface = surface;
        handle = initDecode(width, height);
    }

    public void decodeStream(byte[] data, int length) {
        decodeH264(data, length, handle, mSurface);
    }

    public void close() {
        releaseDecode(handle);
    }

    public native long initDecode(int width, int height);

    public native void decodeH264(byte[] data, int length, long decoder, Surface surface);

    public native void releaseDecode(long seq);

    public native void decodeToYuv(byte[] data, int length, long id, byte[] dst);

//    public native long initHardDecode(Surface surface, int width, int height);
//
//    public native void hardDecode(byte[] data, int length, long decoder);
//
//    public native void releaseHardDecode(long seq);

    static {
        System.loadLibrary("decoding");
    }
}