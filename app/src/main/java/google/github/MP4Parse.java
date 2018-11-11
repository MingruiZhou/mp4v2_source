package google.github;

public class MP4Parse {
    public static final int CREATE_NATIVE_PARSE_FAILED = -1;
    public static final int FILE_HAS_BEEN_OPENED       = -2;

    private long mNativeObj;
    private String mFileName;
    private boolean mIsOpened = false;
    private OnNativeNotify mNotify = null;

    public MP4Parse(String fileName, OnNativeNotify notify){
        mFileName = fileName;
        mNotify   = notify;
    }

    public String getFileName(){
        return mFileName;
    }

    public int openMP4File(){
        if (mIsOpened){
            return FILE_HAS_BEEN_OPENED;
        }

        if (mNativeObj <= 0){
            mNativeObj = _create();  //_create不放构造函数里
            if (mNativeObj <= 0){//创建底层对象失败了就返回错误码
                return CREATE_NATIVE_PARSE_FAILED;
            }
        }

        int ret = _open(mFileName);
        if (ret != 0){
            return ret;
        }

        mIsOpened = true;
        return ret;
    }

    public void closeMP4File(){
        if (mNativeObj != 0){
            _destroy();
        }
    }

    /**
     * 该接口功能仅仅是创建了对应的C++层解析对象，并将对象地址返回
     * @return >0：成功创建了对象； <= 0：错误码
     */
    private native long _create();

    /**
     * 打开MP4文件
     * @param fileName 文件名
     * @return 0：成功；非0：错误码
     */
    private native int _open(String fileName);

    public native byte[] getSPS();

    public native byte[] getPPS();

    public native byte[] getAES();

    public native int getMP4Info();

    private native void _destroy();
    protected void finalize(){
        closeMP4File();
    }


    public void onJNINotifyStopRead(){
        mNotify.onJNINotifyStopRead();
    }

    public void onJNIReadFrame(int type, byte[] frame, long startTime, long duration, long renderingOffset, boolean isSyncSample){
        mNotify.onJNIReadFrame(type, frame, startTime, duration, renderingOffset, isSyncSample);
    }

    /**
     * 开始读取音视频流， 底层开启读取线程， 读取到的音视频流通过回调 onJNIReadFrame 传递到JAVA层
     * @param startTime 开始时间，单位毫秒
     * @param duration 持续时间，单位毫秒
     * @return 0：成功； 其他值为错误码
     */
    public native int start(long startTime, long duration);

    /**
     * 停止音视频流的读取
     */
    public native void stop();

    public interface OnNativeNotify {
        void onJNINotifyStopRead();
        void onJNIReadFrame(int type, byte[] frame, long startTime, long duration, long renderingOffset, boolean isSyncSample);
    }

    /**
     * 获取视频的GOP值
     * @return 0：获取失败或者可能视频流有问题；其他值:正确的GOP值
     */
    public native int getVideoGOP();
}
