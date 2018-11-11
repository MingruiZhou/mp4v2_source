//
// Created by KingMT on 2018/10/18.
// 参考：https://blog.csdn.net/haoyitech/article/details/79589773
//

#ifndef MP4V2_SOURCE_MP4PARSE_H
#define MP4V2_SOURCE_MP4PARSE_H


#include <mp4v2.h>
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
    FILE_RAED_THREAD_START_NO_LISTENER,
    FILE_RAED_THREAD_HAS_STARTED,
    FILE_READ_PARSE_HEADER_FAILED,

    MP4PARSE_RETCODE_END,
};

enum MP4PARSE_READ_THREAD_STATUS:uint8_t {
    THREAD_STATUS_START, //开启线程
    THREAD_STATUS_RECEIVE_CMD_STOP,  //接收到停止线程的命令，
    THREAD_STATUS_EXIT   //线程已经退出，只有原线程已经退出，才能重新开启线程
};

enum MP4PARSE_READ_THREAD_STOP_RET_CODE:uint32_t {
    THREAD_STOP_RET_CODE_END, //读完文件流
    THREAD_STOP_RET_CODE_STOPED,  //被终止
    THREAD_STOP_RET_CODE_ERROR   //线程出错退出
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
    double fps;    //视频帧率
} stVideoInfo;

typedef struct MP4V2_AUDIO_INFO {
    char dataName[64];  //TODO:待确定名字与音频编码类型的关系
    uint32_t timeScale; //音频采样率，单位Hz
    uint64_t duration; //总时长，单位ms
    uint32_t avgBitRate; //平均码率，单位 bps
    int channels;//通道数
} stAudioInfo;

//TODO:具体内容待商议
typedef struct MP4V2_MP4INFO {
    stVideoInfo videoInfo;
    stAudioInfo audioInfo;
} stMP4Info;
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

    void clear(){
        if (m_pData) delete[](m_pData);
        m_pData = nullptr;
        m_size = 0;
    }

    uint32_t size()const {return m_size;};
    const uint8_t* data() const {return m_pData;};
private:
    uint32_t m_size;
    uint8_t* m_pData;
};

/**
 * 流程是  1、 new (Reader*)
 *         2、 openMP4File
 *         3、 getGOP、getSPS、getPPS、getAES
 *         4、 startXXXRead
 *         5、 stop
 *         6、 closeMP4File
 *         7、 delete
 */
class MP4Parse {
private:
    MP4PARSE_RETCODE parseHeader();
    MP4PARSE_RETCODE parseVideoHeader();
    MP4PARSE_RETCODE parseAudioHeader();
    void truncateReadThread();
    void timeLapseReadThread(int speed, bool supportAudio = false);
    MP4SampleId calculateFirstSampleId(MP4TrackId trackId, uint32_t timeScale);
    void addAdts(uint8_t* pBytes, int packetLen);
    MP4SampleId findFirstIFrame();
    void timeLapseReadIFrame(MP4SampleId sampleIdVideo);

public:
    MP4Parse(IDataReader* pDataReader){m_pIDataReader = pDataReader;};
    ~MP4Parse();
    //打开文件、解析文件头、解析文件信息
    MP4PARSE_RETCODE openMP4File(const char* pFileName);

    void closeMP4File();

    //获取视频信息， 暂时用不到
    MP4PARSE_RETCODE getMP4Info(stMP4Info* info);

    /**
     * 获取MP4中h264流的GOP，若为0则表示获取失败且不能进行缩时转录功能
     * @return 0：获取失败；其他值：GOP
     */
    uint32_t getGOP(){return getGOP(m_pMP4FileHandle, m_trackIdVideo);}

    //获取序列参数集
    uint32_t getSPS(const uint8_t** ppSPS)const{ *ppSPS = m_SPS.data(); return m_SPS.size();};

    //获取图像参数集
    uint32_t getPPS(const uint8_t** ppPPS)const{ *ppPPS = m_PPS.data(); return m_PPS.size();};

    //这个是获取音频的什么参数?
    uint32_t getAES(const uint8_t** ppAES)const{ *ppAES = m_AES.data(); return m_AES.size();};

    double getFrameRate(){return MP4GetTrackVideoFrameRate(m_pMP4FileHandle, m_trackIdVideo);}
    /**
     * 开启截取并分离音视频流的线程，不能与其他功能的线程同时执行
     * @param startTime  开始时间， 单位毫秒
     * @param duration  持续时长，为0时表示读到文件未， 单位毫秒
     * @return  SUCCESSED：成功； 其他值：错误码
     */
    MP4PARSE_RETCODE startTruncateRead(uint64_t startTime = 0, uint64_t duration = 0);

    /**
     * 开启倍速读取音视频帧的线程，不能与其他功能的线程同时执行
     * @param speed 倍速， 1为原始速度
     * @param supportAudio 是否支持音频的倍速读取， 暂时不支持音频的倍速读取
     * @return SUCCESSED：成功； 其他值：错误码
     */
    MP4PARSE_RETCODE startTimeLapseRead(int speed, bool supportAudio = false);

    void stop();

    //获取h264流的GOP，如果GOP是变化的， 获取的结果会有误
    static uint32_t getGOP(MP4FileHandle handle, MP4TrackId videoTrack);
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
    uint64_t      m_startTime = 0; //读取流的开始时间，不是非常准确，第一帧数据的时间戳可能比设置的小，因为需要I帧
    uint64_t      m_duration  = 0; //读取流的持续时间， 为0则表示读到文件未
    MP4PARSE_READ_THREAD_STATUS m_bReadingStatus = THREAD_STATUS_EXIT;

    std::thread*  m_pReadFrameThread = nullptr;
    std::mutex    m_ReadFrameThreadmutex;
};

#endif //MP4V2_SOURCE_MP4PARSE_H
