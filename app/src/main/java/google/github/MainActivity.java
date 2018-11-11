package google.github;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import google.mp4v2_github.R;

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
                        (SurfaceView) findViewById(R.id.surfaceView1),
                        "sdcard/test01.h264",
                        "sdcard/test01.aac"),
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file2),
                        (Button) findViewById(R.id.open2),
                        (Button) findViewById(R.id.read2),
                        null,
                        "sdcard/test02.h264",
                        "sdcard/test02.aac"),
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file3),
                        (Button) findViewById(R.id.open3),
                        (Button) findViewById(R.id.read3),
                        null,
                        "sdcard/test03.h264",
                        "sdcard/test03.aac"),
                new LayoutElements(this,
                        (TextView) findViewById(R.id.file4),
                        (Button) findViewById(R.id.open4),
                        (Button) findViewById(R.id.read4),
                        null,
                        "sdcard/test04.h264",
                        "sdcard/test04.aac"),
        };

        verifyStoragePermissions(this);
        MGLog.d(" 生命周期");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
//    public native String stringFromJNI();

    @Override
    protected void onStart(){super.onStart();MGLog.d(" 生命周期");;}
    @Override
    protected void onResume(){super.onResume();MGLog.d(" 生命周期");}
    @Override
    protected void onPause(){super.onPause();MGLog.d(" 生命周期");}
    @Override
    protected void onStop(){super.onStop();MGLog.d(" 生命周期");}
    @Override
    protected void onRestart(){super.onRestart();MGLog.d(" 生命周期");}
    @Override
    protected void onDestroy(){super.onDestroy();MGLog.d(" 生命周期");}

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

    //intent.setType(“image/*”);//选择图片
    //intent.setType(“audio/*”); //选择音频
    //intent.setType(“video/*”); //选择视频 （mp4 3gp 是android支持的视频格式）
    //intent.setType(“video/*;image/*”);//同时选择视频和图片
    private void open(int requestCode){
//        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
//        intent.setType("video/.mp4");
//        intent.addCategory(Intent.CATEGORY_OPENABLE);
//        startActivityForResult(intent,requestCode);

        Intent i = new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Video.Media.EXTERNAL_CONTENT_URI);
        startActivityForResult(i, requestCode);
    }

    public void onClickOpen1(View v){
        if (mLe[0].isOpened()){
            mLe[0].open(null);
        } else {
            open(0);
        }
    }
    public void onClickRead1(View v){
        mLe[0].read();
    }
    public void onClickOpen2(View v){
        if (mLe[1].isOpened()){
            mLe[1].open(null);
        } else {
            open(1);
        }
    }
    public void onClickRead2(View v){
        mLe[1].read();
    }
    public void onClickOpen3(View v){
        if (mLe[2].isOpened()){
            mLe[2].open(null);
        } else {
            open(2);
        }
    }

    private Mp4TimeLapseMuxer mMp4TimeLapseMuxer = new Mp4TimeLapseMuxer();
    public void onClickRead3(View v){
        mLe[2].read();
    }
    public void onClickOpen4(View v){
        if (mLe[3].isOpened()){
            mLe[3].open(null);
        } else {
            open(3);
        }
    }
    public void onClickRead4(View v){
        mLe[3].read();
    }

    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (resultCode == Activity.RESULT_OK) {

            if (requestCode >= 0 && requestCode < 4) {
                Uri uri = data.getData();

//                Toast.makeText(this, "文件路径："+uri.getPath().toString(), Toast.LENGTH_SHORT).show();
                String[] filePathColumn = { MediaStore.Video.Media.DATA };

                Cursor cursor = getContentResolver().query(uri ,
                        filePathColumn, null, null, null);
                cursor.moveToFirst();

                int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
                String videoPath = cursor.getString(columnIndex);
                if (requestCode == 3){
                    if (mMp4TimeLapseMuxer.openSrcFile(videoPath) == 0) {
                        mMp4TimeLapseMuxer.start("sdcard/testMuxer.mp4", 1 * mMp4TimeLapseMuxer.getVideoGOP());//一定要是GOP的帧数倍
                    }
                } else
                    mLe[requestCode].open(videoPath);
                cursor.close();
            }
        }
    }
}
