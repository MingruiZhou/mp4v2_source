package google.mp4v2_github;

import android.widget.Button;
import android.widget.TextView;

public class LayoutElements {
    private TextView mTv = null;
    private Button   mOpenBt = null;
    private Button   mReadBt = null;
    private MP4Parse mParse = null;
    private String mFileName = null;

    public void setInfo(TextView tv, Button open, Button read){
        this.mTv = tv;
        this.mOpenBt = open;
        this.mReadBt = read;
    }

    public int open(){
        if (mParse != null){
            mParse.closeMP4File();
            mParse = null;
            MGLog.d("关闭文件：" + mFileName);
            mOpenBt.setText("打开");
            mIsReading = false;
            mReadBt.setText("读取信息");
            return 0;
        }

        mFileName = mTv.getText().toString();
        mParse = new MP4Parse("sdcard/Download/" + mFileName);
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
                long startTime = 2000;
                long duration = 5000;
                mParse.start(startTime, duration);
                mIsReading = true;
                mReadBt.setText("停止读取");
            }
        }

        return mIsReading ? 1 : 0;
    }
}
