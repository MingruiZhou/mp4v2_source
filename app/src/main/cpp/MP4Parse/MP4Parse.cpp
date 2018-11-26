//
// Created by KingMT on 2018/10/18.
//

#include "MP4Parse.h"
#include <MG_log.h>

MP4Parse::~MP4Parse()
{
    stop();
    if (m_pReadFrameThread)
    {
        m_pReadFrameThread->join();
        delete(m_pReadFrameThread);
        m_pReadFrameThread = nullptr;
    }

    closeMP4File();

    LOGW("------------- destroy MP4Parse, isFileHandleValid:%s, file:%s ------------",
         m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE ? "NO" : "YES", m_fileName.c_str());
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

    MP4PARSE_RETCODE ret = parseHeader();
    if (ret != SUCCESSED)//解析头失败了也算打开失败
    {
        closeMP4File();
        return ret;
    }

    m_fileName.assign(pFileName);
    return SUCCESSED;
}

void MP4Parse::closeMP4File()
{
    LOGD("close MP4File, MP4FileHandle:%p", m_pMP4FileHandle);
    if (m_pMP4FileHandle != MP4_INVALID_FILE_HANDLE){
        MP4Close(m_pMP4FileHandle);
        m_pMP4FileHandle = MP4_INVALID_FILE_HANDLE;
    }

    m_pMP4FileHandle = MP4_INVALID_FILE_HANDLE;
    m_trackIdVideo   = MP4_INVALID_TRACK_ID;
    m_trackIdAudio   = MP4_INVALID_TRACK_ID;
    m_fileName.clear();
    m_SPS.clear();
    m_PPS.clear();
    m_AES.clear();
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

    if (/*m_trackIdAudio != MP4_INVALID_TRACK_ID && //可以没有声音*/m_trackIdVideo != MP4_INVALID_TRACK_ID)
    {
//        LOGW("---------------- 待实现读取数据的线程 -----------------");
        return SUCCESSED;
    }

    return FILE_READ_PARSE_HEADER_FAILED;
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
//                LOGD("SPS:%d %d %d %d %d %d %d %d %d %d %d , size:%u", pSPS[0], pSPS[1], pSPS[2], pSPS[3], pSPS[4], pSPS[5], pSPS[6], pSPS[7], pSPS[8], pSPS[9], pSPS[10], nSize);
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
//                LOGD("PPS:%d %d %d %d , size:%u", pPPS[0], pPPS[1], pPPS[2], pPPS[3], nSize);
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
        LOGE("no mp4 info MP4FileHandle:%s, trackIdVideo:%s, trackIdAudio:%s",
             m_pMP4FileHandle == MP4_INVALID_FILE_HANDLE ? "INVALID" : "valid",
             m_trackIdVideo == MP4_INVALID_TRACK_ID ? "INVALID" : "valid",
             m_trackIdAudio == MP4_INVALID_TRACK_ID ? "INVALID" : "valid");
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

    LOGD("video info => 类型:%s, 每秒刻度数:%u, 总时长:%lu ms, 平均码率:%u bps, 分辨率: %d x %d, 帧率:%f",
         videoInfo.dataName, videoInfo.timeScale, videoInfo.duration, videoInfo.avgBitRate, videoInfo.width, videoInfo.height, videoInfo.fps);
    LOGD("audio info => 类型:%s, 采样率:%u Hz, 总时长:%lu ms, 平均码率:%u bps, 通道数:%d",
         audioInfo.dataName, audioInfo.timeScale, audioInfo.duration, audioInfo.avgBitRate, audioInfo.channels);
    return SUCCESSED;
}

//计算需要的第一帧的MP4SampleId
//视频帧需要从I帧开始读取
MP4SampleId MP4Parse::calculateFirstSampleId(MP4TrackId trackId, uint32_t timeScale)
{
    MP4SampleId sampleId = MP4GetSampleIdFromTime(m_pMP4FileHandle, trackId, timeScale, false);
    for (; sampleId > 1; --sampleId)
    {
        if (MP4GetSampleSync(m_pMP4FileHandle, trackId, sampleId))
            break;
    }
    return sampleId;
}

//TODO：功能初步验证通过，代码待优化，细节待优化
void MP4Parse::addAdts(uint8_t* pBytes, int packetLen)
{
    memset(pBytes, 0x0, 7);
    int profile = MP4GetAudioProfileLevel(m_pMP4FileHandle);  //AAC LC
    int freqIdx = 8;  //16KHz
    int chanCfg = MP4GetTrackAudioChannels(m_pMP4FileHandle, m_trackIdAudio);
    uint32_t audioTimeScale = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdAudio);//音频每秒刻度数(采样率)
    switch (audioTimeScale)
    {
        case 48000:
            freqIdx = 0x03;break;
        case 44100:
            freqIdx = 0x04;break;
        case 32000:
            freqIdx = 0x05;break;
        case 24000:
            freqIdx = 0x06;break;
        case 22050:
            freqIdx = 0x07;break;
        case 16000:
            freqIdx = 0x08;break;
        case 12000:
            freqIdx = 0x09;break;
        case 11025:
            freqIdx = 0x0a;break;
        case 8000:
            freqIdx = 0x0b;break;
    }

    // fill in ADTS data
    pBytes[0] = (unsigned char) 0xFF;
    if (MP4_MPEG4_AUDIO_TYPE != MP4GetTrackEsdsObjectTypeId(m_pMP4FileHandle,m_trackIdAudio ))
        pBytes[1] = (unsigned char) 0xF9;
    else
        pBytes[1] = (unsigned char) 0xF1; //MPEG-4

    pBytes[2] = (unsigned char) (((profile - 1) << 6) + (freqIdx << 2) | (chanCfg & 0x04 >> 2));
    pBytes[3] = (unsigned char) (((chanCfg & 3) << 6) | (packetLen & 0x1800 >> 11));
    pBytes[4] = (unsigned char) ((packetLen & 0x1FF8) >> 3);
    pBytes[5] = (unsigned char) (((packetLen & 7) << 5) | 0x1F);
    pBytes[6] = (unsigned char) 0xFC;
}

void MP4Parse::truncateReadThread()
{
    uint32_t      timeScaleVideo = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdVideo);//视频每秒刻度数; 这个变量用于测试
    uint32_t      timeScaleAudio = MP4GetTrackTimeScale(m_pMP4FileHandle, m_trackIdAudio);//音频每秒刻度数(采样率) 这个变量用于测试

    MP4Timestamp startStampVideo = MP4ConvertToTrackTimestamp(m_pMP4FileHandle, m_trackIdVideo, m_startTime, MP4_MSECS_TIME_SCALE);
    MP4Timestamp   endStampVideo = MP4ConvertToTrackTimestamp(m_pMP4FileHandle, m_trackIdVideo, m_startTime + m_duration, MP4_MSECS_TIME_SCALE);
    MP4SampleId    sampleIdVideo = calculateFirstSampleId(m_trackIdVideo, startStampVideo);
//    MP4SampleId    sampleIdVideo = MP4GetSampleIdFromTime(m_pMP4FileHandle, m_trackIdVideo, startStampVideo, false);

    MP4Timestamp startStampAudio = MP4ConvertToTrackTimestamp(m_pMP4FileHandle, m_trackIdAudio, m_startTime, MP4_MSECS_TIME_SCALE);
    MP4Timestamp   endStampAudio = MP4ConvertToTrackTimestamp(m_pMP4FileHandle, m_trackIdAudio, m_startTime + m_duration, MP4_MSECS_TIME_SCALE);
    MP4SampleId    sampleIdAudio = calculateFirstSampleId(m_trackIdVideo, startStampAudio);
//    MP4SampleId    sampleIdAudio = MP4GetSampleIdFromTime(m_pMP4FileHandle, m_trackIdAudio, startStampAudio, false);

    bool isReadEndVideo = false;
    bool isReadEndAudio = false;

    LOGW("read thread start, Reading:%u, IDataReader:%p, m_startTime:%lu, m_duration:%lu",
         m_bReadingStatus , m_pIDataReader, m_startTime, m_duration);
    LOGI("sampleIdVideo:%u, timeScaleVideo:%u, startStampVideo:%lu", sampleIdVideo, timeScaleVideo, startStampVideo);
    LOGI("sampleIdAudio:%u, timeScaleAudio:%u, startStampAudio:%lu", sampleIdAudio, timeScaleAudio, startStampAudio);

    while(m_bReadingStatus == THREAD_STATUS_START)
    {
        uint8_t* ppBytes = nullptr;
        uint32_t pNumBytes = 0;
        MP4Timestamp startTime = 0;
        MP4Duration  duration = 0;
        MP4Duration  renderingOffset = 0;
        bool isSyncSample = false;

        if (!isReadEndVideo)
        {
//            uint32_t size = MP4GetSampleSize(m_pMP4FileHandle, m_trackIdVideo, sampleIdVideo);
//            ppBytes = new uint8_t[size + ];
            if (MP4ReadSample(m_pMP4FileHandle, m_trackIdVideo, sampleIdVideo, &ppBytes, &pNumBytes,
                              &startTime, &duration, &renderingOffset, &isSyncSample))
            {
                ppBytes[0] = 0x00;
                ppBytes[1] = 0x00;
                ppBytes[2] = 0x00;
                ppBytes[3] = 0x01;
                m_pIDataReader->onRead(STREAM_TYPE_VIDEO, ppBytes, pNumBytes, startTime, duration,
                                       renderingOffset, isSyncSample);
                sampleIdVideo++;
                if (startTime > endStampVideo && m_duration > 0)//视频数据读取完成
                {
                    LOGW("read video sample end!!! firstTime:%lu, lastTime:%lu, timeScaleVideo:%u, duration:%lu",
                         startStampVideo, startTime, timeScaleVideo, m_duration);
                    isReadEndVideo = true;
//                } else {
//                    std::this_thread::sleep_for(std::chrono::milliseconds((duration*1000)/timeScaleVideo));
                }
            } else {
                LOGW("read video sample return false, end read!!!");
                isReadEndVideo = true;
            }

            if (ppBytes)
            {
                MP4Free(ppBytes);
                ppBytes = nullptr;
            }
        }

        if (!isReadEndAudio)
        {
            if (MP4ReadSample(m_pMP4FileHandle, m_trackIdAudio, sampleIdAudio, &ppBytes, &pNumBytes,
                              &startTime, &duration, &renderingOffset, &isSyncSample))
            {
                uint8_t * pAudioFrame = new uint8_t[pNumBytes + 7];
                addAdts(pAudioFrame, pNumBytes + 7);
                memcpy(&pAudioFrame[7] , ppBytes, pNumBytes);
                m_pIDataReader->onRead(STREAM_TYPE_AUDIO, pAudioFrame, pNumBytes + 7, startTime, duration,
                                       renderingOffset, isSyncSample);//
                delete[](pAudioFrame);
                pAudioFrame = nullptr;
                sampleIdAudio++;
                if (startTime > endStampAudio && m_duration > 0)//音频数据读取完成
                {
                    LOGW("read audio sample end!!! firstTime:%lu, lastTime:%lu, timeScaleAudio:%u, duration:%lu",
                         startStampAudio, startTime, timeScaleAudio, m_duration);
                    isReadEndVideo = true;
                }
            } else {
                LOGW("read audio sample return false, end read!!!");
                isReadEndAudio = true;
            }

            if (ppBytes)
            {
                MP4Free(ppBytes);
                ppBytes = nullptr;
            }
        }
        if (isReadEndVideo && isReadEndAudio)
        {
            break;
        }
    };

    m_pIDataReader->onNotifyStoped(m_bReadingStatus == THREAD_STATUS_RECEIVE_CMD_STOP ?
                                   THREAD_STOP_RET_CODE_STOPED : THREAD_STOP_RET_CODE_END);
    LOGW("read thread EXIT, m_bReading:%d, m_pIDataReader:%p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<",
         m_bReadingStatus, m_pIDataReader);
    m_bReadingStatus = THREAD_STATUS_EXIT;
}

// 1、要开启读线程，m_pIDataReader必须不为空
// 2、必须在EXIT状态下才能开启线程
#define CHECK_START_THREAD_CONDITION()  if (m_pIDataReader == nullptr){ \
LOGW("IDataReader is null");\
return FILE_RAED_THREAD_START_NO_LISTENER;\
}\
if (m_bReadingStatus != THREAD_STATUS_EXIT){\
LOGW("reading thread has been started, pleas stop old thread and start a new one");\
return FILE_RAED_THREAD_HAS_STARTED;\
}

MP4PARSE_RETCODE MP4Parse::startTruncateRead(uint64_t startTime, uint64_t duration)
{
    std::lock_guard<std::mutex> lock(m_ReadFrameThreadmutex);
    CHECK_START_THREAD_CONDITION();

    m_startTime = startTime;
    m_duration  = duration;

    m_bReadingStatus = THREAD_STATUS_START;
    if (m_pReadFrameThread) delete(m_pReadFrameThread);
    m_pReadFrameThread = new std::thread([this](){
        this->truncateReadThread();
    });

    return  m_pReadFrameThread != nullptr ? SUCCESSED : FILE_RAED_THREAD_START_FAILED;
};

MP4SampleId MP4Parse::findFirstIFrame()
{
    MP4SampleId maxSampleId = MP4GetTrackNumberOfSamples(m_pMP4FileHandle, m_trackIdVideo);
    MP4SampleId sampleId = 1;
    do
    {
        if (MP4GetSampleSync(m_pMP4FileHandle, m_trackIdVideo, sampleId))
        {
            return sampleId;
        }
        sampleId ++;
    }while(maxSampleId >= sampleId);

    LOGW("************************** there's no I frame ***********************");
    return 0;
}

void MP4Parse::timeLapseReadIFrame(MP4SampleId sampleIdVideo)
{
    uint8_t* ppBytes = nullptr;
    uint32_t pNumBytes = 0;
    MP4Timestamp startTime = 0;
    MP4Duration  duration = 0;
    MP4Duration  renderingOffset = 0;
    bool isSyncSample = false;
    if (MP4ReadSample(m_pMP4FileHandle, m_trackIdVideo, sampleIdVideo, &ppBytes, &pNumBytes,
                      &startTime, &duration, &renderingOffset, &isSyncSample))
    {
        ppBytes[0] = 0x00;
        ppBytes[1] = 0x00;
        ppBytes[2] = 0x00;
        ppBytes[3] = 0x01;
        m_pIDataReader->onRead(STREAM_TYPE_VIDEO, ppBytes, pNumBytes, startTime, duration,
                               renderingOffset, isSyncSample);
        MP4Free(ppBytes);
        ppBytes = nullptr;
    }
}


//此版本会导致P帧丢失，进而导致视频马赛克
#ifdef MOSAIC
void MP4Parse::timeLapseReadThread(int speed, bool supportAudio)
{
    MP4SampleId sampleIdVideo = findFirstIFrame(); // sampleID是从1开始的，保证读的第一帧一定是个I帧，否则不读
    MP4SampleId sampleIdAudio = 1; // sampleID是从1开始的
    bool isReadEndVideo = sampleIdVideo <= 0;
    bool isReadEndAudio = !supportAudio;
    MP4Timestamp nextFrameTimestamp = 0;//下一帧的pts时间
    MP4SampleId iFrameId = 0;
    LOGW("read thread start, Reading:%s, IDataReader:%p, speed:%d, supportAudio:%d, isReadEndVideo:%d, isReadEndAudio:%d",
         m_bReading ? "true" : "false", m_pIDataReader, speed, supportAudio, isReadEndVideo, isReadEndAudio);

    while(m_bReading)
    {
        uint8_t* ppBytes = nullptr;
        uint32_t pNumBytes = 0;
        MP4Timestamp startTime = 0;
        MP4Duration  duration = 0;
        MP4Duration  renderingOffset = 0;
        bool isSyncSample = false;
        if (!isReadEndVideo)
        {//读的第一帧一定是个I帧
            if (MP4ReadSample(m_pMP4FileHandle, m_trackIdVideo, sampleIdVideo, &ppBytes, &pNumBytes,
                              &startTime, &duration, &renderingOffset, &isSyncSample)) {
                LOGD("startTime:%lu, nextFrameTimestamp:%lu, nextFrameTimestamp * speed:%lu",
                     startTime, nextFrameTimestamp, nextFrameTimestamp * speed);
                if (startTime >= nextFrameTimestamp * speed) {
                    if (!isSyncSample && iFrameId != 0) {
                        timeLapseReadIFrame(iFrameId);//先读取IFrame的数据
                        iFrameId = 0;
                    }
                    ppBytes[0] = 0x00;
                    ppBytes[1] = 0x00;
                    ppBytes[2] = 0x00;
                    ppBytes[3] = 0x01;
                    m_pIDataReader->onRead(STREAM_TYPE_VIDEO, ppBytes, pNumBytes, startTime, duration,
                                           renderingOffset, isSyncSample);
                    nextFrameTimestamp += duration;
                } else if (isSyncSample) {
                    iFrameId = sampleIdVideo;
                }

                sampleIdVideo++;
                MP4Free(ppBytes);
                ppBytes = nullptr;
            } else {
                LOGW("read video sample return false, end read!!!");
                isReadEndVideo = true;
            }
        }

        if (!isReadEndAudio)
        {
            if (MP4ReadSample(m_pMP4FileHandle, m_trackIdAudio, sampleIdAudio, &ppBytes, &pNumBytes,
                              &startTime, &duration, &renderingOffset, &isSyncSample))
            {
                m_pIDataReader->onRead(STREAM_TYPE_AUDIO, ppBytes, pNumBytes, startTime, duration,
                                       renderingOffset, isSyncSample);//
                sampleIdAudio++;
            } else {
                LOGW("read audio sample return false, end read!!!");
                isReadEndAudio = true;
            }

            if (ppBytes)
            {
                MP4Free(ppBytes);
                ppBytes = nullptr;
            }
        }
        if (isReadEndVideo && isReadEndAudio)
        {
            break;
        }
    };

    m_pIDataReader->onNotifyStoped();
    LOGW("read thread EXIT, m_bReading:%s, m_pIDataReader:%p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", m_bReading ? "true" : "false", m_pIDataReader);
    m_bReading = false;
}
#else

uint32_t MP4Parse::getGOP(MP4FileHandle handle, MP4TrackId videoTrack)
{
    if (handle == MP4_INVALID_FILE_HANDLE || videoTrack == MP4_INVALID_TRACK_ID)
    {
        LOGE("handle(%p) OR videoTrack(%d) IS INVALID", handle, videoTrack);
        return 0;
    }

    MP4SampleId maxSampleId = MP4GetTrackNumberOfSamples(handle, videoTrack);
    MP4SampleId sampleId = 1;
    MP4SampleId firstIFrameId = 0;
    do
    {
        if (MP4GetSampleSync(handle, videoTrack, sampleId))
        {
            if (firstIFrameId != 0)
            {
                return sampleId - firstIFrameId;
            }

            firstIFrameId = sampleId;
        }
        sampleId ++;
    }while(maxSampleId >= sampleId);

    LOGW("there's no GOP, maxSampleId:%d, sampleId:%d, firstIFrameId:%d ",
         maxSampleId, sampleId, firstIFrameId);
    return 0;
}

void MP4Parse::timeLapseReadThread(int speed, bool supportAudio)
{
    MP4SampleId sampleIdVideo = findFirstIFrame(); // sampleID是从1开始的，保证读的第一帧一定是个I帧，否则不读
    MP4SampleId sampleIdAudio = 1; // sampleID是从1开始的
    bool isReadEndVideo = sampleIdVideo <= 0;
    bool isReadEndAudio = !supportAudio;
    uint32_t gop = getGOP(m_pMP4FileHandle, m_trackIdVideo);
    if (speed % gop){//speed一定要是GOP的整数倍
        m_bReadingStatus = THREAD_STATUS_RECEIVE_CMD_STOP;
        LOGW("speed must be integral multiple of gop");
    }
    LOGW("read thread start, Reading:%u, IDataReader:%p, speed:%d, supportAudio:%d, isReadEndVideo:%d, isReadEndAudio:%d, gop:%u",
         m_bReadingStatus, m_pIDataReader, speed, supportAudio, isReadEndVideo, isReadEndAudio, gop);

    while (m_bReadingStatus == THREAD_STATUS_START) {
        uint8_t* ppBytes = nullptr;
        uint32_t pNumBytes = 0;
        MP4Timestamp startTime = 0;
        MP4Duration  duration = 0;
        MP4Duration  renderingOffset = 0;
        bool isSyncSample = false;
        if (!isReadEndVideo) {
            if (MP4ReadSample(m_pMP4FileHandle, m_trackIdVideo, sampleIdVideo, &ppBytes, &pNumBytes,
                              &startTime, &duration, &renderingOffset, &isSyncSample)) {
                LOGD("startTime:%lu, isSyncSample:%d, sampleIdVideo:%u", startTime, isSyncSample, sampleIdVideo);
                if (isSyncSample) {//只抽取I帧
                    ppBytes[0] = 0x00;
                    ppBytes[1] = 0x00;
                    ppBytes[2] = 0x00;
                    ppBytes[3] = 0x01;
                    m_pIDataReader->onRead(STREAM_TYPE_VIDEO, ppBytes, pNumBytes, startTime, duration,
                                           renderingOffset, isSyncSample);
                } else {
                    LOGW("get a sample that not I frame, maybe this MP4 file contained nonstandard h264 data");
                }
                sampleIdVideo += speed;
            } else {
                LOGW("read video sample return false, end read!!!, sampleIdVideo:%u", sampleIdVideo);
                isReadEndVideo = true;
            }

            if (ppBytes)
                MP4Free(ppBytes);
            ppBytes = nullptr;
        }

        if (!isReadEndAudio)        {
            if (MP4ReadSample(m_pMP4FileHandle, m_trackIdAudio, sampleIdAudio, &ppBytes, &pNumBytes,
                              &startTime, &duration, &renderingOffset, &isSyncSample)) {
                m_pIDataReader->onRead(STREAM_TYPE_AUDIO, ppBytes, pNumBytes, startTime, duration,
                                       renderingOffset, isSyncSample);//
                sampleIdAudio++;
            } else {
                LOGW("read audio sample return false, end read!!!");
                isReadEndAudio = true;
            }

            if (ppBytes) {
                MP4Free(ppBytes);
                ppBytes = nullptr;
            }
        }

        if (isReadEndVideo && isReadEndAudio) {
            break;
        }
    };

    m_pIDataReader->onNotifyStoped(m_bReadingStatus == THREAD_STATUS_RECEIVE_CMD_STOP ?
                                   THREAD_STOP_RET_CODE_STOPED : THREAD_STOP_RET_CODE_END);
    LOGW("read thread EXIT, m_bReading:%d, m_pIDataReader:%p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<",
         m_bReadingStatus, m_pIDataReader);
    m_bReadingStatus = THREAD_STATUS_EXIT;
}
#endif

MP4PARSE_RETCODE MP4Parse::startTimeLapseRead(int speed, bool supportAudio)
{
    std::lock_guard<std::mutex> lock(m_ReadFrameThreadmutex);
    CHECK_START_THREAD_CONDITION();

    m_bReadingStatus = THREAD_STATUS_START;
    if (m_pReadFrameThread) delete(m_pReadFrameThread);
    m_pReadFrameThread = new std::thread([this, speed, supportAudio](){
        this->timeLapseReadThread(speed, supportAudio);
    });

    return  m_pReadFrameThread != nullptr ? SUCCESSED : FILE_RAED_THREAD_START_FAILED;
};

void MP4Parse::stop()
{
    std::lock_guard<std::mutex> lock(m_ReadFrameThreadmutex);
    m_bReadingStatus = THREAD_STATUS_RECEIVE_CMD_STOP;
}