package google.mp4v2_github;

import android.app.Activity;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private LayoutElements [] mLe = null;//new LayoutElements[]{new LayoutElements(), new LayoutElements(), new LayoutElements(), new LayoutElements()};
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mLe = new LayoutElements[]{
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file1),
                        (Button) findViewById(R.id.open1),
                        (Button) findViewById(R.id.read1),
                        (SurfaceView) findViewById(R.id.surfaceView1)),
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file2),
                        (Button) findViewById(R.id.open2),
                        (Button) findViewById(R.id.read2),
                        (SurfaceView) findViewById(R.id.surfaceView1)),
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file3),
                        (Button) findViewById(R.id.open3),
                        (Button) findViewById(R.id.read3),
                        (SurfaceView) findViewById(R.id.surfaceView1)),
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file4),
                        (Button) findViewById(R.id.open4),
                        (Button) findViewById(R.id.read4),
                        (SurfaceView) findViewById(R.id.surfaceView1)),
        };

        verifyStoragePermissions(this);
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    @Override
    protected void onStart(){
        super.onStart();
        MGLog.w("onStart");
    }
    //屏幕方向发生改变的回调方法
    //Manifest中设置了android:configChanges="orientation|keyboardHidden|screenSize" 后在横竖屏切换时就不会销毁重建activity
    //目前Manifest中设置了android:screenOrientation="portrait"  ， 固定为竖屏，所以也不会走到这里来
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            MGLog.w("当前屏幕为横屏");
        } else {
            MGLog.w("当前屏幕为竖屏");
        }
        super.onConfigurationChanged(newConfig);
        //  setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);  //设置横屏
    }

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

    public void onClickOpen1(View v){
        mLe[0].open();
    }
    public void onClickRead1(View v){
        mLe[0].read();
    }
    public void onClickOpen2(View v){
        mLe[1].open();
    }
    public void onClickRead2(View v){
        mLe[1].read();
    }
    public void onClickOpen3(View v){
        mLe[2].open();
    }
    public void onClickRead3(View v){
        mLe[2].read();
    }
    public void onClickOpen4(View v){
        mLe[3].open();
    }
    public void onClickRead4(View v){
        mLe[3].read();
    }
}
