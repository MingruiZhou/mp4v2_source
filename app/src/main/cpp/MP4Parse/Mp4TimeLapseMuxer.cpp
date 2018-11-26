//
// Created by king on 2018/11/7.
//

#include "Mp4TimeLapseMuxer.h"
#include <MG_log.h>

Mp4TimeLapseMuxer::~Mp4TimeLapseMuxer()
{
    if (m_pMp4Muxer)
        delete(m_pMp4Muxer);

    closeSourceFile();

    LOGI("------------ destroy Mp4TimeLapseMuxer --------------");
}

#define DEFAULT_FRAMERATE (15.000000)
//必须先成功调用openSourceFile
int Mp4TimeLapseMuxer::initWriteDestinationFile(const char* desFile)
{
    m_pMp4Muxer = new Mp4Muxer(desFile);
    if (m_pMp4Muxer == nullptr)
    {
        return RET_CODE_NEW_Mp4Muxer_FAILED;
    }

    const uint8_t* sps = nullptr;
    uint32_t spsLen = m_pMP4Parse->getSPS(&sps);
    const uint8_t* pps = nullptr;
    uint32_t ppsLen = m_pMP4Parse->getPPS(&pps);
    double framerate = m_pMP4Parse->getFrameRate();

    if (spsLen > SPS_PPS_DATA_MAX_LEN || ppsLen > SPS_PPS_DATA_MAX_LEN)
    {
        LOGE("SPS or PPS date length error, spsLen:%d, pps:%d, framerate:%f", spsLen, ppsLen, framerate);
        return RET_CODE_SPS_PPS_DATA_LENGTH_ERROR;
    }

    ps_t stSPS = {0}, stPPS = {0};
    stSPS.count = spsLen;
    memcpy(stSPS.data, sps, spsLen);
    stPPS.count = ppsLen;
    memcpy(stPPS.data, pps, ppsLen);

    if (framerate < 1.000000)
        framerate = DEFAULT_FRAMERATE;
    if (!m_pMp4Muxer->SetupVideoTrack(stSPS, stPPS, framerate))
    {
        return RET_CODE_SetupVideoTrack_FAILED;
    }

    return 0;
}

void LogCallback(MP4LogLevel loglevel, const char* fmt, va_list ap)
{
    char str[256] = {0};
    vsprintf(str, fmt, ap);
    LOGW("%d:%s", loglevel, str);
}

int Mp4TimeLapseMuxer::openSourceFile(const char* srcFile)
{
    std::lock_guard<std::mutex> lock(m_mutexLock);
    MP4LogSetLevel(MP4_LOG_VERBOSE4);
    MP4SetLogCallback(LogCallback);
    if (m_pMP4Parse == nullptr)
        m_pMP4Parse = new MP4Parse(this);

    if (m_pMP4Parse == nullptr){
        LOGE("MP4Read file:%s failed!!", srcFile);
        return RET_CODE_NEW_MP4Parse_FAILED;
    }

    return m_pMP4Parse->openMP4File(srcFile);
}

void Mp4TimeLapseMuxer::closeSourceFile()
{
    std::lock_guard<std::mutex> lock(m_mutexLock);
    if (m_pMP4Parse){
        m_pMP4Parse->closeMP4File();
        delete m_pMP4Parse;
        m_pMP4Parse = nullptr;
    }
}

int Mp4TimeLapseMuxer::start(const char* desFile, int speed, bool supportAudio)
{
    std::lock_guard<std::mutex> lock(m_mutexLock);
    LOGD("srcFile:%s, desFile:%s, speed:%d, supportAudio:%d", m_srcFileName.c_str(), desFile, speed, supportAudio);
    if (m_muxerStatus != MUXER_STATUS_STOP){
        return RET_CODE_START_REPEAD;
    }

    int ret = 0;
    do{
        if ((ret = initWriteDestinationFile(desFile)) != SUCCESSED)
            break;

        if ((ret = m_pMP4Parse->startTimeLapseRead(speed , supportAudio)) != SUCCESSED)
            break;

        m_speed = speed;
        m_isSupportAudio = supportAudio;
        m_nextFrameTimestamp = 0;
        m_muxerStatus = MUXER_STATUS_START;
        return ret;
    }while(0);

    //到这里说明前面的步骤执行失败了
    if (m_pMp4Muxer) delete(m_pMp4Muxer), m_pMp4Muxer = nullptr;
    return ret;
}
void Mp4TimeLapseMuxer::stop()
{
    std::lock_guard<std::mutex> lock(m_mutexLock);
    if (m_muxerStatus != MUXER_STATUS_START)
        return ;

    if (m_pMP4Parse)
        m_pMP4Parse->stop();
}

int Mp4TimeLapseMuxer::getGOP()
{
    if (m_pMP4Parse)
        return m_pMP4Parse->getGOP();

    LOGW("MP4Parse is NULL");
    return 0;
}
void Mp4TimeLapseMuxer::onRead(STREAM_TYPE type,
                               const uint8_t* pBytes,
                               uint32_t numBytes,
                               MP4Timestamp startTime,
                               MP4Duration duration,
                               MP4Duration /*renderingOffset*/,
                               uint8_t /*isSyncSample*/)
{
    if (STREAM_TYPE_VIDEO == type)
    {
        if (!m_pMp4Muxer->WriteVideoSample(pBytes, numBytes, m_nextFrameTimestamp))
        {
            LOGE("WriteVideoSample FAILED");
        }
        else
        {
            m_nextFrameTimestamp += duration;
            LOGI("speed:%d, numBytes:%d, time:%lu, startTime:%lu, duration:%lu", m_speed, numBytes, m_nextFrameTimestamp, startTime, duration);
        }
    }
    else if (m_isSupportAudio && STREAM_TYPE_AUDIO == type)
    {
        ;//暂未支持音频的倍速转录
    }

}

void Mp4TimeLapseMuxer::onNotifyStoped(uint32_t retCode)
{
    if (m_pNotification)
        m_pNotification->onStop(retCode);

    std::lock_guard<std::mutex> lock(m_mutexLock);
    if (m_pMP4Parse)
        m_pMP4Parse->closeMP4File();

    if (m_pMp4Muxer)
        delete(m_pMp4Muxer);
    m_pMp4Muxer = nullptr;

    m_muxerStatus = MUXER_STATUS_STOP;
}