//
// Created by kingmt on 2018/10/22.
// 用于创建多线程下的流数据读取器
//

#ifndef MP4V2_SOURCE_IDATAREADER_H
#define MP4V2_SOURCE_IDATAREADER_H

#include <MG_log.h>

enum STREAM_TYPE:int{
    STREAM_TYPE_VIDEO,
    STREAM_TYPE_AUDIO,
};
class IDataReader{
public:
    virtual void onRead( STREAM_TYPE type,
                        const uint8_t* pBytes,
                        uint32_t numBytes,
                        MP4Timestamp startTime,
                        MP4Duration duration,
                        MP4Duration renderingOffset,
                        uint8_t isSyncSample) = 0;

    virtual void onNotifyStoped() = 0;//通知线程已经停止
    virtual ~IDataReader(){LOGI("---------------- destroy IDataReader ---------------");};
};

#endif //MP4V2_SOURCE_IDATAREADER_H
