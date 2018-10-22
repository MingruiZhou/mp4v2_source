//
// Created by KingMT on 2018/10/18.
//

#include "MP4Parse.h"
//#include "MG_log.h"
#include "../MG_log.h"

MP4Parse::~MP4Parse()
{
    if (m_pMP4FileHandle != MP4_INVALID_FILE_HANDLE)
    {
        MP4Close(m_pMP4FileHandle, 0);
    }

    LOGW("------------- destroy MP4Parse, isFileHandleValid:%s, file:%s ------------",
         m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE ? "NO" : "YES", m_fileName.c_str());

    m_pMP4FileHandle = MP4_INVALID_FILE_HANDLE;
    m_fileName.clear();
}
MP4PARSE_RETCODE MP4Parse::openMP4File(const char* pFileName)
{
    if (m_pMP4FileHandle != MP4_INVALID_FILE_HANDLE)
    {
        LOGE("file %s has been opened, please create a new object to open %s", m_fileName.c_str(), pFileName);
        return FILE_HAS_BEEN_OPEND;
    }
    m_pMP4FileHandle = MP4Read(pFileName);
    if (m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE)
    {
        LOGE("MP4Read file:%s failed!!", pFileName);
        return FILE_OPEN_READ_FAILD;
    }

    m_fileName.assign(pFileName);
    return parseHeader();
}

MP4PARSE_RETCODE MP4Parse::parseHeader()
{
    uint32_t trackCount = MP4GetNumberOfTracks(m_pMP4FileHandle, NULL, 0);//获取track数量
    LOGI("get trackCount:%d", trackCount);
    for (uint32_t i = 0; i < trackCount; ++i)
    {
        MP4TrackId  id = MP4FindTrackId(m_pMP4FileHandle, i);
        const char* trackType = MP4GetTrackType(m_pMP4FileHandle, id);
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType))
        {
            m_trackIdVideo = id;
            LOGI("trackId_video:%d, type:%s", m_trackIdVideo, trackType);
            parseVideoHeader();//TODO:parse video header
        }
        else if(MP4_IS_AUDIO_TRACK_TYPE(trackType))
        {
            m_trackIdAudio = id;
            LOGI("trackId_audio:%d, type:%s", m_trackIdAudio, trackType);
            parseAudioHeader();//TODO:parse audio header
        }
        else if (MP4_IS_CNTL_TRACK_TYPE(trackType))
        {
            LOGW("----------------IS_CNTL_TRACK_TYPE-----------------");
        }
        else if (MP4_IS_OD_TRACK_TYPE(trackType))
        {
            LOGW("----------------IS_OD_TRACK_TYPE-----------------");
        }
        else if (MP4_IS_SCENE_TRACK_TYPE(trackType))
        {
            LOGW("----------------IS_SCENE_TRACK_TYPE-----------------");
        }
        else if (MP4_IS_HINT_TRACK_TYPE(trackType))
        {
            LOGW("----------------IS_HINT_TRACK_TYPE-----------------");
        }
        else if (MP4_IS_SYSTEMS_TRACK_TYPE(trackType))
        {
            LOGW("----------------IS_SYSTEMS_TRACK_TYPE-----------------");
        }
    }

    if (m_trackIdAudio != MP4_INVALID_TRACK_ID && m_trackIdVideo != MP4_INVALID_TRACK_ID)
    {
//        LOGW("---------------- 待实现读取数据的线程 -----------------");//TODO:创建读取Sample线程
        return SUCCESSED;
    }

    return FILE_HAS_BEEN_OPEND;
}

MP4PARSE_RETCODE MP4Parse::parseVideoHeader()
{
#ifdef MP4_DEBUG
    char* pVideoInfo = MP4Info(m_pMP4FileHandle, m_trackIdVideo);//视频信息
    uint32_t videoTimeScale = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdVideo);//视频每秒刻度数
    const char * videoName = MP4GetTrackMediaDataName(m_pMP4FileHandle, m_trackIdVideo); //视频编码

    LOGI("mp4 video info:%s, track time scale:%u, media data name:%s", pVideoInfo == NULL ? "get video info failed" : pVideoInfo,
         videoTimeScale, videoName == NULL ? "no name" : videoName);

    MP4Free(pVideoInfo);
#endif
    uint8_t** spsHeader = NULL;
    uint32_t* spsSize = NULL;

    uint8_t** ppsHeader = NULL;
    uint32_t* ppsSize = NULL;

    if (MP4GetTrackH264SeqPictHeaders(m_pMP4FileHandle, m_trackIdVideo, &spsHeader, &spsSize, &ppsHeader, &ppsSize))
    {
        for (uint32_t ix = 0; spsSize[ix] != 0; ++ix)
        {
            uint8_t * pSPS = spsHeader[ix];// SPS指针
            uint32_t  nSize = spsSize[ix];// SPS长度

            if (m_SPS.size() <= 0 && nSize > 0)// 只存储第一个SPS信息...
            {
                m_SPS.assign(pSPS, nSize);
            }
            MP4Free(spsHeader[ix]);
            LOGI("=== 找到第 %d 个SPS格式头，长度：%u ===", ix+1, nSize);
        }
        MP4Free(spsHeader);
        MP4Free(spsSize);

        for (uint32_t ix = 0; ppsSize[ix] != 0; ++ix)
        {
            uint8_t * pPPS = ppsHeader[ix]; // PPS指针
            uint32_t  nSize = ppsSize[ix];//和长度

            if (m_PPS.size() <= 0 && nSize > 0)// 只存储第一个PPS信息...
            {
                m_PPS.assign(pPPS, nSize);
            }
            free(ppsHeader[ix]);
            LOGI("=== 找到第 %d 个PPS格式头，长度：%u ===", ix+1, nSize);
        }
        MP4Free(ppsHeader);
        MP4Free(ppsSize);

        return SUCCESSED;
    }

    return FILE_MP4GetTrackH264SeqPictHeaders_FAILED;
}

MP4PARSE_RETCODE MP4Parse::parseAudioHeader()
{
#ifdef MP4_DEBUG
    char* pAudioInfo = MP4Info(m_pMP4FileHandle, m_trackIdAudio);//音频信息
    uint32_t audioTimeScale = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdAudio);//音频每秒刻度数(采样率)
    const char * audioName = MP4GetTrackMediaDataName(m_pMP4FileHandle, m_trackIdAudio); //音频编码
    uint8_t	audioType = MP4GetTrackAudioMpeg4Type(m_pMP4FileHandle, m_trackIdAudio); //音频的格式类型
    int audioChannels = MP4GetTrackAudioChannels(m_pMP4FileHandle, m_trackIdAudio); //音频声道数
    uint32_t audioRateIndex = 0; //音频采样率编号
    switch (audioTimeScale)
    {
        case 48000:
            audioRateIndex = 0x03;break;
        case 44100:
            audioRateIndex = 0x04;break;
        case 32000:
            audioRateIndex = 0x05;break;
        case 24000:
            audioRateIndex = 0x06;break;
        case 22050:
            audioRateIndex = 0x07;break;
        case 16000:
            audioRateIndex = 0x08;break;
        case 12000:
            audioRateIndex = 0x09;break;
        case 11025:
            audioRateIndex = 0x0a;break;
        case 8000:
            audioRateIndex = 0x0b;break;
    }
    LOGI("mp4 audio info:%s, audio track time scale:%u, media data name:%s, audioType:%u, audioChannels:%d, "
         "audioRateIndex:%u", pAudioInfo == NULL ? "get audio info failed" : pAudioInfo,
         audioTimeScale, audioName == NULL ? "no name" : audioName, audioType, audioChannels, audioRateIndex);

    MP4Free(pAudioInfo);
#endif
    uint8_t * pAES = NULL;
    uint32_t  nSize = 0;
    if (MP4GetTrackESConfiguration(m_pMP4FileHandle, m_trackIdAudio, &pAES, &nSize))
    {
        if (m_AES.size() <= 0 && nSize > 0)
        {// 存储音频扩展信息...
            m_AES.assign(pAES, nSize);
        }
        LOGI("=== 音频扩展信息长度：%u ===", nSize);
        MP4Free(pAES);// 释放分配的缓存...
        return SUCCESSED;
    }

    return FILE_MP4GetTrackESConfiguration_FAILED;
}

MP4PARSE_RETCODE MP4Parse::getMP4Info(stMP4Info* info)
{
//    char* pVideoInfo = MP4Info(m_pMP4FileHandle, m_trackIdVideo);//视频信息
//    MP4Free(pVideoInfo);
    if (m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE ||
        m_trackIdVideo == MP4_INVALID_TRACK_ID ||
        m_trackIdAudio == MP4_INVALID_TRACK_ID)
    {
        LOGE("no mp4 info");
        return FILE_NOT_OPEN;
    }

    /****************************** video info ******************************/
    stVideoInfo& videoInfo = info->videoInfo;
    const char * videoName = MP4GetTrackMediaDataName(m_pMP4FileHandle, m_trackIdVideo); //视频编码
    if (videoName)
    {
        memcpy(videoInfo.dataName, videoName, sizeof(videoInfo.dataName));
    }

    if ((strcasecmp(videoName, "avc1") == 0) ||
        (strcasecmp(videoName, "264b") == 0)) //avc
    {
        MP4GetTrackH264ProfileLevel(m_pMP4FileHandle, m_trackIdVideo,
                                    &videoInfo.profile, &videoInfo.level);
    }
    videoInfo.timeScale = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdVideo);//视频每秒刻度数
    MP4Duration videoDuration = MP4GetTrackDuration(m_pMP4FileHandle, m_trackIdVideo);
    videoInfo.duration = MP4ConvertFromTrackDuration(m_pMP4FileHandle, m_trackIdVideo, videoDuration, MP4_MSECS_TIME_SCALE);
    videoInfo.avgBitRate = MP4GetTrackBitRate(m_pMP4FileHandle, m_trackIdVideo); //平均码率，单位 bps
    videoInfo.width = MP4GetTrackVideoWidth(m_pMP4FileHandle, m_trackIdVideo); //视频宽
    videoInfo.height = MP4GetTrackVideoHeight(m_pMP4FileHandle, m_trackIdVideo);; //视频高
    videoInfo.fps = MP4GetTrackVideoFrameRate(m_pMP4FileHandle, m_trackIdVideo);  //视频帧率

    /****************************** audio info ******************************/
    stAudioInfo& audioInfo = info->audioInfo;
    const char * audioName = MP4GetTrackMediaDataName(m_pMP4FileHandle, m_trackIdAudio); //音频编码
    if (audioName)
    {
        memcpy(audioInfo.dataName, audioName, sizeof(audioInfo.dataName));
    }
    audioInfo.timeScale  = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdAudio);//音频每秒刻度数(采样率)
    MP4Duration audioDuration = MP4GetTrackDuration(m_pMP4FileHandle, m_trackIdAudio);
    audioInfo.duration   = MP4ConvertFromTrackDuration(m_pMP4FileHandle, m_trackIdAudio, audioDuration, MP4_MSECS_TIME_SCALE);//转毫秒
    audioInfo.avgBitRate = MP4GetTrackBitRate(m_pMP4FileHandle, m_trackIdAudio); //单位 bps
    audioInfo.channels   = MP4GetTrackAudioChannels(m_pMP4FileHandle, m_trackIdAudio); //音频声道数

    LOGD("video info => 类型:%s, 每秒刻度数:%u, 总时长:%llu ms, 平均码率:%u bps, 分辨率: %d x %d, 帧率:%f",
         videoInfo.dataName, videoInfo.timeScale, videoInfo.duration, videoInfo.avgBitRate, videoInfo.width, videoInfo.height, videoInfo.fps);
    LOGD("audio info => 类型:%s, 采样率:%u Hz, 总时长:%llu ms, 平均码率:%u bps, 通道数:%d",
         audioInfo.dataName, audioInfo.timeScale, audioInfo.duration, audioInfo.avgBitRate, audioInfo.channels);
    return SUCCESSED;
}

void MP4Parse::readDataThread()
{
    LOGW("read thread start, m_bReading:%s, m_pIDataReader:%p >>>>>>>>>>>>>>>>>>>>>>>>>>>>>", m_bReading ? "true" : "false", m_pIDataReader);
    int test = 0;
    uint8_t testData[] = {0,1,2,3,4,5,6,7,8,9,'a','b','c','d','e'};
    while(m_bReading)
    {
        test++;
        m_pIDataReader->read(test%2 == 0 ? STREAM_TYPE_VIDEO : STREAM_TYPE_AUDIO, testData,
                             sizeof(testData), 777, 888, 999, test%2);//TODO:读数据
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    m_pIDataReader->notifyStoped();
    m_bReading = false;
    LOGW("read thread EXIT, m_bReading:%s, m_pIDataReader:%p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", m_bReading ? "true" : "false", m_pIDataReader);
}

MP4PARSE_RETCODE MP4Parse::start(MP4Timestamp startTime, MP4Timestamp duration)
{
    std::lock_guard<std::mutex> lock(m_ReadFrameThreadmutex);
    if (m_bReading)
    {
        LOGW("reading thread has been started, pleas stop old thread and start a new one");
        return FILE_RAED_THREAD_HAS_STARTED;
    }
    m_startTime = startTime;
    m_duration = duration;

    m_bReading = true;
    if (m_pReadFrameThread) delete(m_pReadFrameThread);
    m_pReadFrameThread = new std::thread([this](){
        this->readDataThread();
    });

    m_bReading = m_pReadFrameThread != nullptr;
    return  m_bReading ? FILE_RAED_THREAD_START_FAILED : SUCCESSED;
};

void MP4Parse::stop()
{
    std::lock_guard<std::mutex> lock(m_ReadFrameThreadmutex);
    m_bReading = false;
    if (m_pReadFrameThread)
    {
        m_pReadFrameThread->join();
        delete(m_pReadFrameThread);
    }

    m_pReadFrameThread = nullptr;
}