package google.mp4v2_github;

import android.app.Activity;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.view.SurfaceView;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class LayoutElements implements MP4Parse.OnNativeNotify{
    private Activity mActivity;
    private TextView mTv = null;
    private Button   mOpenBt = null;
    private Button   mReadBt = null;
    private MP4Parse mParse = null;
    private String mFileName = null;

    MediaCodec mMediaCodec = null;
    SurfaceView mSurfaceView = null;

    public LayoutElements(Activity activity, TextView tv, Button open,
                          Button read, SurfaceView surfaceView){
        this.mTv = tv;
        this.mOpenBt = open;
        this.mReadBt = read;
        this.mActivity = activity;
        this.mSurfaceView = surfaceView;
    }

    public void setInfo(Activity activity, TextView tv, Button open, Button read){
        this.mTv = tv;
        this.mOpenBt = open;
        this.mReadBt = read;
        this.mActivity = activity;
    }

    public int open(){
        if (mParse != null){
            mParse.closeMP4File();
            mParse = null;
            MGLog.d("关闭文件：" + mFileName);
            mOpenBt.setText("打开");
            mIsReading = false;
            if (mMediaCodec != null) {
                mMediaCodec.stop();
                mMediaCodec.release();
            }

            mReadBt.setText("读取信息");
            return 0;
        }

        mFileName = mTv.getText().toString();
        mParse = new MP4Parse("sdcard/Download/" + mFileName, this);
        int ret = mParse.openMP4File();
        MGLog.d("打开文件：" + mFileName + ", ret：" + ret);
        mOpenBt.setText("关闭");
        return 0;
    }

    boolean mIsReading = false;
    public int read(){
        if (mParse != null){
            if (mIsReading){
                MGLog.i("开始停止读取。。。。。。。");
                mParse.stop();
                mIsReading = false;
                MGLog.i("停止读取结束。。。。。。。。。");
                mReadBt.setText("读取信息");
            } else {
                byte[] sps = mParse.getSPS();
                byte[] pps = mParse.getPPS();
                byte[] aes = mParse.getAES();
                MGLog.i("sps:" + sps + ", spsLen:" + (sps != null ? sps.length : 0) + ", pps:" + pps + ", ppsLen:" + (pps != null ? pps.length : 0) +
                        ", aes:" + aes + ", aesLen:" + (aes != null ? aes.length : 0));
                mParse.getMP4Info();
                initDecoder();//
                mParse.start(0, 0);
                mIsReading = true;
                mReadBt.setText("停止读取");
            }
        }

        return mIsReading ? 1 : 0;
    }

    // Video Constants
    private final static String MIME_TYPE = "video/avc"; // H.264 Advanced Video
    private final static int VIDEO_WIDTH = 1920;
    private final static int VIDEO_HEIGHT = 1088;
    private final static int TIME_INTERNAL = 30;
    private final static int TIMEOUT_US = 1000;

    public void initDecoder() {
        try {
            mMediaCodec = MediaCodec.createDecoderByType(MIME_TYPE);
            mBufferInfo = new MediaCodec.BufferInfo();
        }catch (IOException e) {
            e.printStackTrace();
        }

//        int[] width = new int[1];
//        int[] height = new int[1];
//        AvcUtils.parseSPS(mParse.getSPS(), width, height);//从sps中解析出视频宽高
        MediaFormat mediaFormat = MediaFormat.createVideoFormat(MIME_TYPE, VIDEO_WIDTH, VIDEO_HEIGHT);
        mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(mParse.getSPS()));
        mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(mParse.getPPS()));
        mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, VIDEO_WIDTH * VIDEO_HEIGHT);
        mMediaCodec.configure(mediaFormat, mSurfaceView.getHolder().getSurface(),
                null, 0);
        mMediaCodec.start();
    }

    @Override
    public void onJNINotifyStopRead(){
        MGLog.w("文件名：" + mFileName + ", 停止读取数据");
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mReadBt.setText("读取信息");
            }
        });

        try {
            mSaveFile.close();
        }catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        mFirstDecord = true;
    }

    private boolean mFirstDecord = true;
    MediaCodec.BufferInfo mBufferInfo;
    public void onJNIReadFrame(int type, byte[] frame, long startTime, long duration, long renderingOffset, boolean isSyncSample){
//        MGLog.i("文件名：" + mFileName + ", frameLength:" + frame.length + ",streamType:" + type + ", startTime:" + startTime + ", duration:" + duration +
//                ", renderingOffset:" + renderingOffset + ", isSyncSample:" + isSyncSample);
        if (type == 1){
            return;
        }
        //TODO：存文件
        saveFrameToFile(frame, isSyncSample);
//        byte[] displayFrame = null;
//        if (mFirstDecord){
//            displayFrame = new byte[4*2 + mParse.getSPS().length + mParse.getPPS().length + frame.length];
//            displayFrame[0] = 0x0;
//            displayFrame[1] = 0x0;
//            displayFrame[2] = 0x0;
//            displayFrame[3] = 0x1;
//            System.arraycopy(mParse.getSPS(), 0, displayFrame, 4, mParse.getSPS().length);
////            displayStream(displayFrame, 0);
//
//            displayFrame = new byte[4 + mParse.getPPS().length];
//            displayFrame[mParse.getSPS().length + 4 + 0] = 0x0;
//            displayFrame[mParse.getSPS().length + 4 + 1] = 0x0;
//            displayFrame[mParse.getSPS().length + 4 + 2] = 0x0;
//            displayFrame[mParse.getSPS().length + 4 + 3] = 0x1;
//            System.arraycopy(mParse.getSPS(), 0, displayFrame, 4 + mParse.getSPS().length + 4, mParse.getPPS().length);
//            System.arraycopy(frame, 0, displayFrame, 4 + mParse.getSPS().length + 4 + mParse.getPPS().length, frame.length);
//            displayStream(displayFrame, 0);
//            mFirstDecord = false;
//            return;
//        }

        displayStream(frame, duration);
    }

    private void displayStream(byte[] frame, long duration){
        int inputBufferIndex = mMediaCodec.dequeueInputBuffer(TIMEOUT_US);
        if (inputBufferIndex >= 0) {
            ByteBuffer buffer = mMediaCodec.getInputBuffer(inputBufferIndex);
            buffer.clear();
            buffer.put(frame, 0, frame.length);

            try{
                mMediaCodec.queueInputBuffer(inputBufferIndex, 0, frame.length, duration, 0);
            }catch (MediaCodec.CryptoException e){
                MGLog.e("queueInputBuffer erro");
            }

        } else {
            MGLog.e("dequeueInputBuffer return a invalid index:" + inputBufferIndex);
            return;
        }

        // Get output buffer index
        int outIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, TIMEOUT_US);
        while (outIndex >= 0) {
            ByteBuffer outputBuffer = mMediaCodec.getOutputBuffer(outIndex);
            outputBuffer.position(mBufferInfo.offset);
            outputBuffer.limit(mBufferInfo.offset + mBufferInfo.size);
            mMediaCodec.releaseOutputBuffer(outIndex, true);
            outIndex = mMediaCodec.dequeueOutputBuffer(mBufferInfo, TIMEOUT_US);// 再次获取数据，如果没有数据输出则outIndex=-1
        }
    }

    private String mSaveFileVideo = "sdcard/test5.h264";
    private String mSaveFileAudio = "sdcard/test5.aac";
    private FileOutputStream mSaveFile;

    private void saveFrameToFile(byte[] frame, boolean isSyncSample){
        try{
            if (mSaveFile == null) {
                mSaveFile = new FileOutputStream(mSaveFileVideo);
            }

            if (isSyncSample){
                byte[] header = {0, 0, 0, 1};
                mSaveFile.write(header);
                mSaveFile.write(mParse.getSPS());
                mSaveFile.write(header);
                mSaveFile.write(mParse.getPPS());
            }
            mSaveFile.write(frame);
        }catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
