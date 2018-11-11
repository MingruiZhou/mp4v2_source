//
// Created by King on 2018/11/7.
// MP4的缩时录制，未转码
//

#ifndef MP4V2_SOURCE_MP4TIMELAPSEMUXER_H
#define MP4V2_SOURCE_MP4TIMELAPSEMUXER_H

#include <mp4v2.h>
#include "Mp4Muxer.h"
#include "IDataReader.h"
#include "MP4Parse.h"

enum MP4TIME_LAPSE_MUXER_RETCODE:int{
    RET_CODE_START_REPEAD = MP4PARSE_RETCODE_END,
    RET_CODE_NEW_MP4Parse_FAILED,
    RET_CODE_NEW_Mp4Muxer_FAILED,
    RET_CODE_SPS_PPS_DATA_LENGTH_ERROR,
    RET_CODE_SetupVideoTrack_FAILED,
    RET_CODE_openSourceFile_REPEAD,
};

typedef enum TIME_LAPSE_MUXER_STATUS{
    MUXER_STATUS_START, //开始
    MUXER_STATUS_STOP,  //停止
} enMuxerStatus;

class IMuxerNotification{
public:
    virtual void onStop(uint32_t retCode) = 0;
    virtual ~IMuxerNotification(){};
};

/**
 *  大致流程：1、new (IMuxerNotification)
 *            2、openSourceFile
 *            3、getGOP
 *            4、start
 *            5、stop
 *            6、closeSourceFile
 *            7、delete
 */
class Mp4TimeLapseMuxer : public IDataReader{
public:
    Mp4TimeLapseMuxer(IMuxerNotification* pNotification):
            m_pNotification(pNotification){};
    ~Mp4TimeLapseMuxer();
public:
    /**
     * 打开源文件
     * @param srcFile 源文件名
     * @return 0：打开成功；其他值：错误码
     */
    int openSourceFile(const char* srcFile);

    /**
     * 关闭打开的源文件
     */
    void closeSourceFile();
    /**
     * 开始转录
     * @param speed  相对于原始速度的倍速
     * @param supportAudio   是否支持音频的转录，目前暂不支持音频
     * @return 0:成功开启转录线程；其他值：错误码
     */
    int start(const char* desFile, int speed, bool supportAudio = false);
    void stop();
    int getGOP();
public:
    IMuxerNotification* getNotification(){return m_pNotification;};
    virtual void onRead( STREAM_TYPE type,
                         const uint8_t* pBytes,
                         uint32_t numBytes,
                         MP4Timestamp startTime,
                         MP4Duration duration,
                         MP4Duration renderingOffset,
                         uint8_t isSyncSample);

    virtual void onNotifyStoped(uint32_t retCode);//通知线程已经停止

private:
    int initReadSourceFile(const char* srcFile);
    int initWriteDestinationFile(const char* desFile);
    void doMuxerThread();
private:
    IMuxerNotification* m_pNotification;
    MP4Parse* m_pMP4Parse = nullptr;
    Mp4Muxer* m_pMp4Muxer = nullptr;
    int m_muxerStatus = MUXER_STATUS_STOP;
    std::string m_srcFileName;
    int m_speed = 1; //倍速, 1为原始速度
    bool m_isSupportAudio = false;
    MP4Timestamp m_nextFrameTimestamp = 0;//下一帧的pts时间
    std::mutex m_mutexLock;
};


#endif //MP4V2_SOURCE_MP4TIMELAPSEMUXER_H
