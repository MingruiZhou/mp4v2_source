package google.mp4v2_github;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        verifyStoragePermissions(this);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    private static final int REQUEST_EXTERNAL_STORAGE = 1;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };
    public static void verifyStoragePermissions(Activity activity) {
        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE,REQUEST_EXTERNAL_STORAGE);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    MP4Parse m_MP4Parse;
    public void parseFile(View v){
        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.fileName);
        String fileName = tv.getText().toString();

        if (m_MP4Parse == null || !m_MP4Parse.getmFileName().equals(fileName)){
            m_MP4Parse = null;
            m_MP4Parse = new MP4Parse(fileName);
        }

        int ret = m_MP4Parse.openMP4File();
        MGLog.d("打开文件：" + fileName + ", ret：" + ret);
    }

    private boolean mIsReadingFrame = false;
    public void getMP4Info(View v) {

        if (m_MP4Parse != null){
            if (mIsReadingFrame){
                MGLog.i("开始停止读取。。。。。。。");
                m_MP4Parse.stop();
                mIsReadingFrame = false;
                MGLog.i("停止读取结束。。。。。。。。。");
                return;
            }
            byte[] sps = m_MP4Parse.getSPS();
            byte[] pps = m_MP4Parse.getPPS();
            byte[] aes = m_MP4Parse.getAES();
            MGLog.i("sps:" + sps + ", spsLen:" + (sps != null ? sps.length : 0 ) + ", pps:" + pps + ", ppsLen:" + (pps != null ? pps.length : 0 )+
                    ", aes:" + aes + ", aesLen:" + (aes != null ? aes.length : 0 ));
            m_MP4Parse.getMP4Info();
            m_MP4Parse.start(2000, 5000);
            mIsReadingFrame = true;
        }
    }
}
