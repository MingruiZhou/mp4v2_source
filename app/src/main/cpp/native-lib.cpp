#include <jni.h>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <MG_log.h>
#include <MP4Parse.h>

jlong get_parse_obj_from_java(JNIEnv *env, jobject thiz){
    jclass jclas = env->GetObjectClass(thiz);
    do {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("call GetObjectClass failed, ExceptionCheck return true!");
            break;
        }

        if (nullptr == jclas) {
            LOGE("call GetObjectClass failed, retval is NULL");
            break;
        }

        jfieldID fieldId = env->GetFieldID(jclas, "mNativeObj", "J");
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("call GetFieldID failed, ExceptionCheck return true!");
            break;
        }

        if (nullptr == fieldId) {
            LOGE("call GetFieldID failed, retval is NULL");
            break;
        }

        env->DeleteLocalRef(jclas);
        return env->GetLongField(thiz, fieldId);
    }while(0);

    if (jclas != nullptr){
        env->DeleteLocalRef(jclas);
    }
    return 0;
}

void destroy_parse_obj_from_java(JNIEnv *env, jobject thiz){
    jclass jclas = env->GetObjectClass(thiz);
    do {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("call GetObjectClass failed, ExceptionCheck return true!");
            break;
        }

        if (nullptr == jclas) {
            LOGE("call GetObjectClass failed, retval is NULL");
            break;
        }

        jfieldID fieldId = env->GetFieldID(jclas, "mNativeObj", "J");
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("call GetFieldID failed, ExceptionCheck return true!");
            break;
        }

        if (nullptr == fieldId) {
            LOGE("call GetFieldID failed, retval is NULL");
            break;
        }

        env->SetLongField(thiz, fieldId, 0);
    }while(0);

    if (jclas != nullptr){
        env->DeleteLocalRef(jclas);
    }
}

void set_parse_obj_to_java(JNIEnv *env, jobject thiz, jlong obj){
    jclass jclas = env->GetObjectClass(thiz);
    do {
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("call GetObjectClass failed, ExceptionCheck return true!");
            break;
        }

        if (nullptr == jclas) {
            LOGE("call GetObjectClass failed, retval is NULL");
            break;
        }

        jfieldID fieldId = env->GetFieldID(jclas, "mNativeObj", "J");
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            LOGE("call GetFieldID failed, ExceptionCheck return true!");
            break;
        }

        if (nullptr == fieldId) {
            LOGE("call GetFieldID failed, retval is NULL");
            break;
        }

        env->SetLongField(thiz, fieldId, obj);
    }while(0);

    if (jclas != nullptr){
        env->DeleteLocalRef(jclas);
    }
}
/*
unsigned char sps[64],pps[64];
int spslen = 0,ppslen = 0;

void printInfo(MP4FileHandle hFile, MP4TrackId videoTrackId)
{
    MP4Duration Duration      = MP4GetDuration(hFile);
    uint32_t TimeScale        = MP4GetTimeScale(hFile);
    uint8_t ODProfileLevel    = MP4GetODProfileLevel(hFile);
    uint8_t SceneProfileLevel = MP4GetSceneProfileLevel(hFile);
    uint8_t VideoProfileLevel = MP4GetVideoProfileLevel(hFile, videoTrackId);
    uint8_t AudioProfileLevel = MP4GetAudioProfileLevel(hFile);
    uint8_t GraphicsProfileLevel = MP4GetGraphicsProfileLevel(hFile);
    uint16_t GetAmrModeSet    = MP4GetAmrModeSet(hFile, videoTrackId);
    const char *GetHrefTrackBaseUrl = MP4GetHrefTrackBaseUrl (hFile, videoTrackId);
    LOGI("Duration:%llu, TimeScale:%u, ODProfileLevel:%d, SceneProfileLevel:%d, VideoProfileLevel:%d, "
                 "AudioProfileLevel:%d, GraphicsProfileLevel:%d, GetAmrModeSet:%d, GetHrefTrackBaseUrl:%s",
         Duration, TimeScale, ODProfileLevel, SceneProfileLevel, VideoProfileLevel, AudioProfileLevel,
         GraphicsProfileLevel, GetAmrModeSet, GetHrefTrackBaseUrl == nullptr ? "NULL" : GetHrefTrackBaseUrl);
    ;
    ;
    ;
    ;
    ;
    ;
    ;
}

int get264stream(MP4FileHandle oMp4File,int VTrackId,int totalFrame)
{
    if(!oMp4File) return -1;
    char NAL[5] = {0x00,0x00,0x00,0x01};
    unsigned char *pData = NULL;
    unsigned int nSize = 0;
    MP4Timestamp pStartTime;
    MP4Duration pDuration;
    MP4Duration pRenderingOffset;
    bool pIsSyncSample = 0;

    int nReadIndex = 0;
    FILE *pFile = NULL;
    pFile = fopen("/sdcard/out17.h264", "wb");

    while(nReadIndex < totalFrame)
    {
        nReadIndex ++;
        //LOGD("nReadIndex:%d\n",nReadIndex);
        MP4ReadSample(oMp4File,VTrackId,nReadIndex,&pData,&nSize,&pStartTime,&pDuration,&pRenderingOffset,&pIsSyncSample);

        //IDR֡ 帧，写入sps pps先
        if(pIsSyncSample)
        {
            fwrite(NAL, 4, 1, pFile);
            fwrite(sps, spslen, 1, pFile);

            fwrite(NAL, 4, 1, pFile);
            fwrite(pps, ppslen, 1, pFile);
        }
        //264frame
        if(pData && nSize > 4)
        {
            //标准的264帧，前面几个字节就是frame的长度.
            //需要替换为标准的264 nal 头.
            pData[0] = 0x00;
            pData[1] = 0x00;
            pData[2] = 0x00;
            pData[3] = 0x01;
            fwrite(pData,nSize,1,pFile);
        }

        //如果传入MP4ReadSample的视频pData是null
        // 它内部就会new 一个内存
        //如果传入的是已知的内存区域，
        //则需要保证空间bigger then max frames size.
        free(pData);
        pData = NULL;
    }
    fflush(pFile);
    fclose(pFile);

    return 0;
}
int openmp4file(const char *sMp4file)
{
    MP4FileHandle oMp4File;
    int i;

    //unsigned int oStreamDuration;
    unsigned int oFrameCount;

    oMp4File = MP4Read(sMp4file);
    int videoindex = -1,audioindex = -1;//记录videoTrackID、audioTrackID
    uint32_t numSamples;
    //uint32_t timescale;
    //uint64_t duration;

    if (oMp4File == MP4_INVALID_FILE_HANDLE)
    {
        LOGE("Read error....%s\r\n", sMp4file);
        return -1;
    }

    MP4TrackId trackId = MP4_INVALID_TRACK_ID;
    uint32_t numTracks = MP4GetNumberOfTracks(oMp4File, NULL, 0);//获取track数量
    LOGI("numTracks:%d\n", numTracks);

    for (i = 0; i < numTracks; i++)
    {
        trackId = MP4FindTrackId(oMp4File, i, NULL, 0);//通过索引获取track ID
        const char* trackType = MP4GetTrackType(oMp4File, trackId);//通过track ID 获取track类型
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType))//判断track类型
        {
            videoindex= trackId;//记录videoTrackID
            numSamples = MP4GetTrackNumberOfSamples(oMp4File, trackId);
            //timescale = MP4GetTrackTimeScale(oMp4File, trackId);
            //oStreamDuration = duration/(timescale/1000);
            oFrameCount = numSamples;

            // read sps/pps
            uint8_t **seqheader;
            uint8_t **pictheader;
            uint32_t *pictheadersize;
            uint32_t *seqheadersize;
            uint32_t ix;
            MP4GetTrackH264SeqPictHeaders(oMp4File, trackId, &seqheader, &seqheadersize, &pictheader, &pictheadersize);

            for (ix = 0; seqheadersize[ix] != 0; ix++)
            {
                memcpy(sps, seqheader[ix], seqheadersize[ix]);
                spslen = seqheadersize[ix];
                free(seqheader[ix]);
            }
            free(seqheader);
            free(seqheadersize);

            for (ix = 0; pictheadersize[ix] != 0; ix++)
            {
                memcpy(pps, pictheader[ix], pictheadersize[ix]);
                ppslen = pictheadersize[ix];
                free(pictheader[ix]);
            }
            free(pictheader);
            free(pictheadersize);
        }
        else if (MP4_IS_AUDIO_TRACK_TYPE(trackType))
        {
            audioindex = trackId;
            LOGI("audioindex:%d\n",audioindex);
        }
    }

    //解析完了mp4，主要是为了获取sps pps 还有video的trackID
    if(videoindex >= 0)
        get264stream(oMp4File,videoindex,oFrameCount);

    printInfo(oMp4File, videoindex);
    //需要mp4close 否则在嵌入式设备打开mp4上多了会内存泄露挂掉.
    MP4Close(oMp4File, 0);
    return 0;
}
*/
void testOpenFile(const char* fileName)
{
    FILE* pTestFile = fopen(fileName, "wb");
    if (pTestFile)
    {
        LOGI("open file %s successed!", fileName);
        fclose(pTestFile);
    }
    else
    {
        LOGE("OPEN FILE %s FAILED!!", fileName);
    }

    return ;
}

/*********************************      测试MP4Parse用        **********************************/
#include <IDataReader.h>
#include <MP4Parse/Mp4TimeLapseMuxer.h>

class TestParse:public IDataReader{
public:
    bool initEvn(JNIEnv *pEnv, jobject thiz){
        jint ret = pEnv->GetJavaVM(&m_JVM);
        if (pEnv->ExceptionCheck()) {
            pEnv->ExceptionDescribe();
            pEnv->ExceptionClear();
            LOGE("call GetFieldID failed, ExceptionCheck return true!");
            return false;
        }

        m_Java_MP4Parse_GlobleRef = pEnv->NewGlobalRef(thiz);
        m_JNI_VERSION = pEnv->GetVersion();
        return m_JVM != NULL && m_Java_MP4Parse_GlobleRef != NULL;
    }

    virtual void onRead( STREAM_TYPE type,
                         const uint8_t* pBytes,
                         uint32_t numBytes,
                         MP4Timestamp startTime,
                         MP4Duration duration,
                         MP4Duration renderingOffset,
                         uint8_t isSyncSample){
        if (!m_isStart){
            if (m_JVM == NULL){
                LOGE("m_JVM is NULL!");
                return ;
            }

            if (m_JVM->GetEnv((void**) &m_pThreadEnv, m_JNI_VERSION) != JNI_OK)
            {
//                LOGE("get JNI env failed! m_JNI_VERSION = %d", m_JNI_VERSION);
                if (m_JVM->AttachCurrentThread(&m_pThreadEnv, NULL) != JNI_OK){
                    LOGE("AttachCurrentThread failed!");
                    return ;
                }
            }

            jclass cls = m_pThreadEnv->GetObjectClass(m_Java_MP4Parse_GlobleRef);
            if(!cls){
                LOGE("GetObjectClass failed, m_Java_MP4Parse_GlobleRef:%p", m_Java_MP4Parse_GlobleRef);
                return ;
            }

            m_readMethodID = m_pThreadEnv->GetMethodID(cls, "onJNIReadFrame", "(I[BJJJZ)V");
            if(!m_readMethodID){
                LOGE("GetMethodID of onJNIReadFrame failed, m_Java_MP4Parse_GlobleRef:%p", m_Java_MP4Parse_GlobleRef);
                return ;
            }

            m_notifyMethodID = m_pThreadEnv->GetMethodID(cls, "onJNINotifyStopRead", "()V");
            if(!m_notifyMethodID){
                LOGE("GetMethodID of onJNINotifyStopRead failed, m_Java_MP4Parse_GlobleRef:%p", m_Java_MP4Parse_GlobleRef);
                return ;
            }

            m_isStart = true;
        }

        jbyteArray frameData = m_pThreadEnv->NewByteArray(numBytes);//TODO：这种方式效率低
//        m_pThreadEnv->NewDirectByteBuffer(pBytes, numBytes);
        m_pThreadEnv->SetByteArrayRegion(frameData, 0, numBytes, (jbyte*)pBytes);
        m_pThreadEnv->CallVoidMethod(m_Java_MP4Parse_GlobleRef, m_readMethodID,
                                     type, frameData, startTime, duration, renderingOffset, isSyncSample);
        m_pThreadEnv->DeleteLocalRef(frameData);
#ifdef DEBUG_SAVE_FILE
        if (type == STREAM_TYPE_VIDEO)
            saveToFile(pBytes, numBytes, isSyncSample);
#endif
    };

    //在与JAVA的交互中必须且只能在调用onRead的线程中使用
    virtual void onNotifyStoped(uint32_t retCode){
        m_pThreadEnv->CallVoidMethod(m_Java_MP4Parse_GlobleRef, m_notifyMethodID);
        m_JVM->DetachCurrentThread();
        m_isStart = false;
#ifdef DEBUG_SAVE_FILE
        fclose(m_pFile);
        m_pFile = NULL;
#endif
    };

    ~TestParse(){
        if (m_Java_MP4Parse_GlobleRef){
            JNIEnv* pEnv = NULL;
            int ret = m_JVM->GetEnv((void**)&pEnv, m_JNI_VERSION);
            pEnv->DeleteGlobalRef(m_Java_MP4Parse_GlobleRef);
        }
        LOGI("======================== destroy TestParse =====================");
    };
#ifdef DEBUG_SAVE_FILE
    FILE* m_pFile = NULL;//已经验证OK，不要用APP录制的视频做实验
    virtual void saveToFile(const uint8_t* pBytes, uint32_t numBytes, uint8_t isKey){
        if (m_pFile == NULL){
            m_pFile = fopen("/sdcard/out66.h264", "wb");
            if (m_pFile == NULL){
                LOGE("open file failed");
                return;
            }
        }

        if(isKey)
        {
            MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(m_pThreadEnv, m_Java_MP4Parse_GlobleRef);
            const uint8_t *pData = NULL;
            uint32_t size = pMP4Parse->getSPS(&pData);
            char NAL[5] = {0x00,0x00,0x00,0x01};
            fwrite(NAL, 4, 1, m_pFile);
            fwrite(pData, size, 1, m_pFile);

//            LOGD("SPS:%d %d %d %d %d %d %d %d %d %d %d , size:%u", pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7], pData[8], pData[9], pData[10], size);
            pData = NULL;
            size = pMP4Parse->getPPS(&pData);
            fwrite(NAL, 4, 1, m_pFile);
            fwrite(pData, size, 1, m_pFile);
//            LOGD("PPS:%d %d %d %d , size:%u", pData[0], pData[1], pData[2], pData[3], size);
        }
        //264frame
        if(pBytes && numBytes > 4)
        {
            //标准的264帧，前面几个字节就是frame的长度.
            //需要替换为标准的264 nal 头.
            fwrite(pBytes, numBytes, 1, m_pFile);
        }

    }
#endif
private:
    bool m_isStart = false;
    jobject m_Java_MP4Parse_GlobleRef = NULL;
    jmethodID m_readMethodID = NULL;
    jmethodID m_notifyMethodID = NULL;
    JavaVM* m_JVM = NULL;
    JNIEnv* m_pThreadEnv = NULL;
    jint m_JNI_VERSION = JNI_VERSION_1_6;
};

/*********************************      测试MP4Parse用    END **********************************/
enum NATIVE_RETCODE:int{
    NATIVE_ENV_ERROR = MP4PARSE_RETCODE_END,
};

extern "C" JNIEXPORT jstring
JNICALL
Java_google_github_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "/sdcard/20181015162811.mp4";
//    testOpenFile(hello.c_str());
//    openmp4file(hello.c_str());
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jlong
JNICALL
Java_google_github_MP4Parse__1create(
        JNIEnv *env,
        jobject thiz) {
    TestParse* TP = new TestParse();
    if (TP != nullptr && TP->initEvn(env, thiz)){
        return (jlong) (new MP4Parse(TP));
    }

    if (TP) delete(TP);
    LOGE("TestParse:%p, init failed! ", TP);
    return (jlong)0;
}

extern "C" JNIEXPORT jint
JNICALL
Java_google_github_MP4Parse__1open(
        JNIEnv *env, jobject thiz,
        jstring fileName) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    const char* file = env->GetStringUTFChars(fileName, nullptr);
//    openmp4file(file);//已经验证OK，不要用APP录制的视频做实验
//    testOpenFile(file);
    if (pMP4Parse)
        return pMP4Parse->openMP4File(file);

    return NATIVE_ENV_ERROR;
}

extern "C" JNIEXPORT void
JNICALL
Java_google_github_MP4Parse__1destroy(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        pMP4Parse->stop();
        TestParse* testParse = dynamic_cast<TestParse*>(pMP4Parse->getDataReader());
        if (testParse){
            delete(testParse);
        }
        delete(pMP4Parse);
        destroy_parse_obj_from_java(env, thiz);
    } else {
        LOGE("NO DELETE MP4Parse OBJECT");
    }
}

extern "C" JNIEXPORT jbyteArray
JNICALL
Java_google_github_MP4Parse_getSPS(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        const uint8_t* pData = NULL;
        uint32_t ret = pMP4Parse->getSPS(&pData);//TODO:这里可能不能转string，因为SPS、PPS、AES并不是字符串
        if (ret > 0){
            jbyteArray arr = env->NewByteArray(ret);
            const jbyte* p = (const jbyte*)(pData);
            env->SetByteArrayRegion(arr, 0, ret, const_cast<jbyte*>(p));
            return arr;
        }

        LOGE("ret = %d, pData:%p", ret, pData);
        return NULL;
    }

    return NULL;
}

extern "C" JNIEXPORT jbyteArray
JNICALL
Java_google_github_MP4Parse_getPPS(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        const uint8_t* pData = NULL;
        uint32_t ret = pMP4Parse->getPPS(&pData);//TODO:这里可能不能转string，因为SPS、PPS、AES并不是字符串
        if (ret > 0){
            jbyteArray arr = env->NewByteArray(ret);
            const jbyte* p = (const jbyte*)(pData);
            env->SetByteArrayRegion(arr, 0, ret, const_cast<jbyte*>(p));
            return arr;
        }

        LOGE("ret = %d, pData:%p", ret, pData);
        return NULL;
    }

    return NULL;
}

extern "C" JNIEXPORT jbyteArray
JNICALL
Java_google_github_MP4Parse_getAES(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        const uint8_t* pData = NULL;
        uint32_t ret = pMP4Parse->getAES(&pData);//TODO:这里可能不能转string，因为SPS、PPS、AES并不是字符串
        if (ret > 0){
            jbyteArray arr = env->NewByteArray(ret);
            const jbyte* p = (const jbyte*)(pData);
            env->SetByteArrayRegion(arr, 0, ret, const_cast<jbyte*>(p));
            return arr;
        }

        LOGE("ret = %d, pData:%p", ret, pData);
        return NULL;
    }

    return NULL;
}

extern "C" JNIEXPORT jint
JNICALL
Java_google_github_MP4Parse_getMP4Info(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        stMP4Info info = {0};
        return pMP4Parse->getMP4Info(&info);
    }

    return NATIVE_ENV_ERROR;
}

extern "C" JNIEXPORT jint
JNICALL
Java_google_github_MP4Parse_start(
        JNIEnv *env,
        jobject thiz,
        jlong startTime, jlong duration) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        LOGI("startTime:%ld, duration:%ld", startTime, duration);
        return pMP4Parse->startTruncateRead(startTime, duration);
    }

    return NATIVE_ENV_ERROR;
}

extern "C" JNIEXPORT void
JNICALL
Java_google_github_MP4Parse_stop(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        pMP4Parse->stop();
    }
}

extern "C" JNIEXPORT jint
JNICALL
Java_google_github_MP4Parse_getVideoGOP(
        JNIEnv *env,
        jobject thiz) {
    MP4Parse* pMP4Parse = (MP4Parse*)get_parse_obj_from_java(env, thiz);
    if (pMP4Parse){
        return pMP4Parse->getGOP();
    }

    return 0;
}

class TestMuxer:public IMuxerNotification{
    virtual void onStop(uint32_t retCode){
        LOGW("已经停止转录。。。。");
    }
};

extern "C" JNIEXPORT jlong
JNICALL
Java_google_github_Mp4TimeLapseMuxer_create(
        JNIEnv *env,
        jobject thiz) {
    TestMuxer* TM = new TestMuxer();
    if (TM != nullptr){
        return (jlong) (new Mp4TimeLapseMuxer(TM));
    }

    return 0L;
}

extern "C" JNIEXPORT void
JNICALL
Java_google_github_Mp4TimeLapseMuxer_destroy(
        JNIEnv *env,
        jobject thiz) {
    Mp4TimeLapseMuxer* pMp4TimeLapseMuxer = (Mp4TimeLapseMuxer*)get_parse_obj_from_java(env, thiz);
    if (pMp4TimeLapseMuxer){
        destroy_parse_obj_from_java(env, thiz);
        pMp4TimeLapseMuxer->stop();
        TestMuxer* TM = dynamic_cast<TestMuxer*>(pMp4TimeLapseMuxer->getNotification());
        delete(pMp4TimeLapseMuxer);
        delete(TM);
    }
}


extern "C" JNIEXPORT jint
JNICALL
Java_google_github_Mp4TimeLapseMuxer_openSourceFile(
        JNIEnv *env,
        jobject thiz,
        jstring srcFile) {
    jboolean iscopy = false;
    const char* pSrcFile = env->GetStringUTFChars(srcFile, &iscopy);
    Mp4TimeLapseMuxer* pMp4TimeLapseMuxer = (Mp4TimeLapseMuxer*)get_parse_obj_from_java(env, thiz);
    if (pMp4TimeLapseMuxer){
        return pMp4TimeLapseMuxer->openSourceFile(pSrcFile);
    }

    return -1;
}

extern "C" JNIEXPORT void
JNICALL
Java_google_github_Mp4TimeLapseMuxer_closeSourceFile(
        JNIEnv *env,
        jobject thiz) {
    Mp4TimeLapseMuxer* pMp4TimeLapseMuxer = (Mp4TimeLapseMuxer*)get_parse_obj_from_java(env, thiz);
    if (pMp4TimeLapseMuxer){
        pMp4TimeLapseMuxer->closeSourceFile();
    }
}

extern "C" JNIEXPORT jint
JNICALL
Java_google_github_Mp4TimeLapseMuxer_start(
        JNIEnv *env,
        jobject thiz,
        jstring desFile, jint speed) {
    Mp4TimeLapseMuxer* pMp4TimeLapseMuxer = (Mp4TimeLapseMuxer*)get_parse_obj_from_java(env, thiz);
    if (pMp4TimeLapseMuxer){
        jboolean iscopy = false;

        const char* pDesFile = env->GetStringUTFChars(desFile, &iscopy);
        return pMp4TimeLapseMuxer->start(pDesFile, speed);
    }

    return -1;
}

extern "C" JNIEXPORT void
JNICALL
Java_google_github_Mp4TimeLapseMuxer_stop(
        JNIEnv *env,
        jobject thiz) {
    Mp4TimeLapseMuxer* pMp4TimeLapseMuxer = (Mp4TimeLapseMuxer*)get_parse_obj_from_java(env, thiz);
    if (pMp4TimeLapseMuxer){
        pMp4TimeLapseMuxer->stop();
    }
}

extern "C" JNIEXPORT jint
JNICALL
Java_google_github_Mp4TimeLapseMuxer_getVideoGOP(
        JNIEnv *env,
        jobject thiz) {
    Mp4TimeLapseMuxer* pMp4TimeLapseMuxer = (Mp4TimeLapseMuxer*)get_parse_obj_from_java(env, thiz);
    if (pMp4TimeLapseMuxer){
        return pMp4TimeLapseMuxer->getGOP();
    }

    return 0;
}
