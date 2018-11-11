package google.github;

public class Mp4TimeLapseMuxer {
    private long mNativeObj = 0;
    public int openSrcFile(String srcFile){
        if (mNativeObj != 0){
            closeSourceFile();
        } else {
            mNativeObj = create();
            if (0 == mNativeObj){
                MGLog.e("create native object failed");
                return -1;
            }
        }

        return openSourceFile(srcFile);
    }

    public void closeSrcFile(){
        closeSourceFile();
        destroy();
    }

    private native long create();
    private native void destroy();
    private native int openSourceFile(String srcFile);
    private native void closeSourceFile();
    public native int start(String desFile, int speed);
    public native void stop();
    /**
     * 获取视频的GOP值
     * @return 0：获取失败或者可能视频流有问题；其他值:正确的GOP值
     */
    public native int getVideoGOP();
}
