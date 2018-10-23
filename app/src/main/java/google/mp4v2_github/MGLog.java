package google.mp4v2_github;


import android.util.Log;

public class MGLog {
    private static int mLeavelV = 0;
    private static int mLeavelD = 1;
    private static int mLeavelI = 2;
    private static int mLeavelW = 3;
    private static int mLeavelE = 4;

    private static int mLeavel = -1;

    private static String filter = "";
    public static void setLeavel(int leavel){
        mLeavel = leavel;
    }

    public static void v(String log){
        if (mLeavelV > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.v(tag, "[" + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log);
        }
    }

    public static void d(String log){
        if (mLeavelD > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.d(tag, "[" + filter + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log);
        }
    }

    public static void i(String log){
        if (mLeavelI > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.i(tag, "[" + filter + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log);
        }
    }

    public static void w(String log){
        if (mLeavelW > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.w(tag, "[" + filter + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log);
        }
    }

    public static void e(String log){
        if (mLeavelE > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.e(tag, "[" + filter + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log);
        }
    }

    public static void e(String log, Throwable e){
        if (mLeavelE > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.e(tag, "[" + filter + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log, e);
        }
    }

    public static void wtf(String log){
        if (mLeavelE > mLeavel){
            String tag = "("  + Thread.currentThread().getStackTrace()[3].getFileName() + ":" + Thread.currentThread().getStackTrace()[3].getLineNumber() + ")";
            Log.wtf(tag, "[" + filter + Thread.currentThread().getStackTrace()[3].getMethodName() + "]" + log);
        }
    }
}
