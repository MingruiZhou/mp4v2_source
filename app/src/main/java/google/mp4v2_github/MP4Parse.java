package google.mp4v2_github;

public class MP4Parse {
    public static final int CREATE_NATIVE_PARSE_FAILED = -1;
    public static final int FILE_HAS_BEEN_OPENED       = -2;


    private long mNativeMP4Parse;
    private String mFileName;
    private boolean mIsOpened = false;

    public MP4Parse(String fileName){
        mFileName = fileName;
    }

    public String getmFileName(){
        return mFileName;
    }

    public int openMP4File(){
        if (mIsOpened){
            return FILE_HAS_BEEN_OPENED;
        }

        if (mNativeMP4Parse <= 0){
            mNativeMP4Parse = _create();  //_create不放构造函数里
            if (mNativeMP4Parse <= 0){//创建底层对象失败了就返回错误码
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
        _destroy();
    }


    public void onJNINotifyStopRead(){
        MGLog.w("停止读取数据");
    }

    public void onJNIReadFrame(int type, byte[] frame, long startTime, long duration, long renderingOffset, boolean isSyncSample){
        MGLog.i("streamType:" + type + ", startTime:" + startTime + ", duration:" + duration +
                ", renderingOffset:" + renderingOffset + ", isSyncSample:" + isSyncSample);
        //TODO：存文件
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
}
