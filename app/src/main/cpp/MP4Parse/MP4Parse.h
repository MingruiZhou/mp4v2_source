//
// Created by KingMT on 2018/10/18.
// 参考：https://blog.csdn.net/haoyitech/article/details/79589773
//

#ifndef MP4V2_SOURCE_MP4PARSE_H
#define MP4V2_SOURCE_MP4PARSE_H


#include <include/mp4v2/mp4v2.h>
#include <string>
#include <thread>

#include "IDataReader.h"
#define MP4_DEBUG

enum MP4PARSE_RETCODE:int{
    SUCCESSED = 0,
    FILE_NOT_OPEN,
    FILE_HAS_BEEN_OPEND,
    FILE_OPEN_READ_FAILD, //在打开的时候读失败
    FILE_MP4GetTrackH264SeqPictHeaders_FAILED,
    FILE_MP4GetTrackESConfiguration_FAILED,
    FILE_RAED_THREAD_START_FAILED,
    FILE_RAED_THREAD_HAS_STARTED,

    MP4PARSE_RETCODE_END,
};

extern "C" {//提供C接口
typedef struct MP4V2_VIDEO_INFO {
    char dataName[64];  //"avc1" 和 "264b" 是h264流
    uint8_t profile;   //264的属性
    uint8_t level;     //264的属性
    uint32_t timeScale; //视频每秒刻度数
    uint64_t duration;    //视频总时长，单位ms
    uint32_t avgBitRate;  //平均码率，单位 bps
    uint16_t width;  //视频宽
    uint16_t height; //视频高
    double  fps;    //视频帧率
}stVideoInfo;

typedef struct MP4V2_AUDIO_INFO {
    char dataName[64];  //TODO:待确定名字与音频编码类型的关系
    uint32_t timeScale; //音频采样率，单位Hz
    uint64_t duration; //总时长，单位ms
    uint32_t avgBitRate; //平均码率，单位 bps
    int      channels;//通道数
}stAudioInfo;

//TODO:具体内容待商议
typedef struct MP4V2_MP4INFO {
    stVideoInfo videoInfo;
    stAudioInfo audioInfo;
} stMP4Info;

//TODO:具体参数待确认
typedef void (*pReadCallBackFuntion) (const uint8_t* pBytes,
                                        uint32_t numBytes,
                                        MP4Timestamp startTime,
                                        MP4Duration duration,
                                        MP4Duration renderingOffset,
                                        uint8_t isSyncSample);

}

class HeaderData{
public:
    HeaderData():m_size(0), m_pData(nullptr){};
    ~HeaderData(){if (m_pData){ delete(m_pData); }};
    void assign(const uint8_t* pData, uint32_t size){
        if (m_pData) delete[](m_pData);
        m_pData = new uint8_t[size];
        memcpy(m_pData, pData, size);
        m_size = size;
    };

    uint32_t size()const {return m_size;};
    const uint8_t* data() const {return m_pData;};
private:
    uint32_t m_size;
    uint8_t* m_pData;
};

/**
 * 流程是  1、 new (Reader)
 *         2、 openMP4File
 *         3、 getSPS、getPPS、getAES
 *         4、 setReadTime
 *         5、 start
 *         6、 stop
 *         7、 delete
 */
class MP4Parse {
private:
    MP4PARSE_RETCODE parseHeader();
    MP4PARSE_RETCODE parseVideoHeader();
    MP4PARSE_RETCODE parseAudioHeader();
    void readDataThread();

public:
    MP4Parse(IDataReader* pDataReader){m_pIDataReader = pDataReader;};
    ~MP4Parse();
    //打开文件、解析文件头、解析文件信息
    MP4PARSE_RETCODE openMP4File(const char* pFileName);

    //获取视频信息， 暂时用不到
    MP4PARSE_RETCODE getMP4Info(stMP4Info* info);

    //获取序列参数集
    uint32_t getSPS(const uint8_t** ppSPS)const{ *ppSPS = m_SPS.data(); return m_SPS.size();};

    //获取图像参数集
    uint32_t getPPS(const uint8_t** ppPPS)const{ *ppPPS = m_PPS.data(); return m_PPS.size();};

    //这个是获取音频的什么参数?
    uint32_t getAES(const uint8_t** ppAES)const{ *ppAES = m_AES.data(); return m_AES.size();};

    /*
     * 开启读取数据
     * startTime：开始时间， 单位毫秒
     * duration ：持续时长，为0时表示读到文件未， 单位毫秒
     */
    MP4PARSE_RETCODE start(MP4Timestamp startTime = 0, MP4Timestamp duration = 0);

    void stop();


public:
    IDataReader* getDataReader(){return m_pIDataReader;};
private:
    MP4FileHandle m_pMP4FileHandle = MP4_INVALID_FILE_HANDLE;
    MP4TrackId    m_trackIdVideo   = MP4_INVALID_TRACK_ID;
    MP4TrackId    m_trackIdAudio   = MP4_INVALID_TRACK_ID;
    IDataReader*  m_pIDataReader = nullptr;
    std::string   m_fileName;
    HeaderData    m_SPS; //不可以使用string类型存储
    HeaderData    m_PPS;
    HeaderData    m_AES;
    MP4Timestamp  m_startTime = 0;
    MP4Timestamp  m_duration  = 0;
    bool          m_bReading = false;

    std::thread*  m_pReadFrameThread = nullptr;
    std::mutex    m_ReadFrameThreadmutex;
};


#endif //MP4V2_SOURCE_MP4PARSE_H
