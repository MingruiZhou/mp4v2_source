#include <malloc.h>
#include <include/mp4v2/mp4v2.h>
#include "Mp4Muxer.h"
#include "tool.h"
#include "../MG_log.h"
#include "sps_decode.h"

#define TICKS 90000

Mp4Muxer::Mp4Muxer(const char *filename) :
        mMp4FileHandle(MP4_INVALID_FILE_HANDLE),
        mVideoTrackId(MP4_INVALID_TRACK_ID),
        mAudioTrackId(MP4_INVALID_TRACK_ID),
        mSampleRate(44100),
        mLastVideoTimestampUs(0), mLastAudioTimestampUs(0), mFrameCounter(0),
        mFilename(filename) {
    LOGD(" %s MP4:%s", __FUNCTION__, filename);
    mMp4FileHandle = MP4CreateEx(filename, 0, 1, 1, 0, 0, 0, 0);

    if (mMp4FileHandle == MP4_INVALID_FILE_HANDLE) {
        LOGE(" %s 创建MP4文件:%s失败", __FUNCTION__, filename);
    }
}

Mp4Muxer::~Mp4Muxer() {
    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE) {
        if (mVideoTrackId != MP4_INVALID_TRACK_ID) {
            CheatFrameRate();
        }
        MP4Close(mMp4FileHandle);
    } else {
        LOGE(" %s 关闭MP4文件失败", __FUNCTION__);
    }
}

void Mp4Muxer::CheatFrameRate() {
    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE && mVideoTrackId != MP4_INVALID_TRACK_ID) {
        MP4Duration dur = MP4GetTrackDuration(mMp4FileHandle, mVideoTrackId);
        int sc = MP4GetTrackNumberOfSamples(mMp4FileHandle, mVideoTrackId);
        int fc = dur * mVideoFrameRate / TICKS - sc;
        for (int i = 0; i < fc; i++) {
            MP4WriteSample(mMp4FileHandle, mVideoTrackId, NULL, 0, 0, 0, 0);
        }
    }

}

string Mp4Muxer:: GetFileName() {
    return mFilename;
}

uint64_t Mp4Muxer::GetVideoDuration(){
    if(mVideoTrackId == MP4_INVALID_TRACK_ID)
        return 0;
    MP4Duration sampleDuration = MP4GetTrackDuration(mMp4FileHandle, mVideoTrackId);
   // uint32_t timescale = MP4GetTrackTimeScale(mMp4FileHandle, mVideoTrackId);
   // if(timescale<=0)
   //     timescale = TICKS;
    //u_int64_t myDuration = sampleDuration/(timescale/1000);//MP4ConvertFromTrackDuration(mMp4FileHandle, mVideoTrackId,sampleDuration, timescale);
    u_int64_t myDuration =  MP4ConvertFromTrackDuration(mMp4FileHandle, mVideoTrackId,sampleDuration, MP4_MSECS_TIME_SCALE);
    return myDuration;
}

uint64_t Mp4Muxer::GetAudioDuration(){
    if(mAudioTrackId == MP4_INVALID_TRACK_ID)
        return 0;
    MP4Duration sampleDuration = MP4GetTrackDuration(mMp4FileHandle, mAudioTrackId);
   // uint32_t timescale = MP4GetTrackTimeScale(mMp4FileHandle, mAudioTrackId);
   // if(timescale<=0)
   //     timescale = mSampleRate;
   // u_int64_t myDuration = sampleDuration/(timescale/1000);//MP4ConvertFromTrackDuration(mMp4FileHandle, mAudioTrackId,sampleDuration, timescale);
    u_int64_t myDuration = MP4ConvertFromTrackDuration(mMp4FileHandle, mAudioTrackId,sampleDuration, MP4_MSECS_TIME_SCALE);
    return myDuration;
}

bool Mp4Muxer::Check() {
    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE &&
        (mVideoTrackId != MP4_INVALID_TRACK_ID || mAudioTrackId != MP4_INVALID_TRACK_ID)) {
        return true;
    } else {
        return false;
    }
}

bool Mp4Muxer::SetupAudioTrack(int32_t sampleRate, int32_t fixedDuration,const uint8_t* pConfig,uint32_t configSize) {
    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE) {
        mSampleRate = sampleRate;

        /* 备注:
         * MP4AddAudioTrack的第四个参数要设置为MP4_MPEG4_AUDIO_TYPE
         * 原先设置为MP4_MPEG2_AAC_LC_AUDIO_TYPE，在ios用原生播放器
         * 无法播放!!!
         */
        mAudioTrackId = MP4AddAudioTrack(mMp4FileHandle, sampleRate, fixedDuration,
                                         MP4_MPEG4_AUDIO_TYPE);
        if (mAudioTrackId == MP4_INVALID_TRACK_ID) {
            LOGE(" %s 增加音频 Track失败", __FUNCTION__);
        } else {
            MP4SetAudioProfileLevel(mMp4FileHandle, 0x02);
            MP4SetTrackESConfiguration(mMp4FileHandle, mAudioTrackId, pConfig,configSize);
            return true;
        }
    } else {
        LOGE(" %s MP4文件句柄无效", __FUNCTION__);
    }
    return false;
}

bool Mp4Muxer::WriteAudioSample(uint8_t *aac, int64_t size, int64_t timestamp) {
    if(size < 8)
        return false;

    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE) {
        if (mAudioTrackId != MP4_INVALID_TRACK_ID) {
            MP4Duration ttt = MP4_INVALID_DURATION;
#if 0
            ttt = timestamp*mSampleRate/1000;
#endif

            bool b = MP4WriteSample(mMp4FileHandle, mAudioTrackId, aac + 7, size - 7,ttt);
            mAudioTrackId = b ? mAudioTrackId : MP4_INVALID_TRACK_ID;
            return true;
        } else {
            LOGE(" %s 音频Track无效", __FUNCTION__);
        }
    } else {
        LOGE(" %s MP4文件句柄无效", __FUNCTION__);
    }
    return false;
}

#define DEFAULT_FRAMERATE  (30)
bool Mp4Muxer::SetupVideoTrack(int32_t width, int32_t height, int32_t framerate,
                                    uint8_t *sps_pps_data, int64_t data_count) {
    if( framerate <= 0 )
        framerate = DEFAULT_FRAMERATE;
    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE) {
        MP4SetTimeScale(mMp4FileHandle, TICKS);
        ps_t sps;
        ps_t pps;
        parse_sps_pps(sps_pps_data, data_count, sps, pps);
        mVideoTrackId = MP4AddH264VideoTrack(mMp4FileHandle, TICKS, TICKS / framerate, width,
                                             height,
                                             sps.data[1], //sps[1] AVCProfileIndication
                                             sps.data[2], //sps[2] profile_compat
                                             sps.data[3], //sps[3] AVCLevelIndication
                                             3); // 4 bytes length before each NAL unit

        if (mVideoTrackId == MP4_INVALID_TRACK_ID) {
            LOGE(" %s 增加H264Track失败", __FUNCTION__);
        } else {
            MP4AddH264SequenceParameterSet(mMp4FileHandle, mVideoTrackId, sps.data, sps.count);
            MP4AddH264PictureParameterSet(mMp4FileHandle, mVideoTrackId, pps.data, pps.count);
            MP4SetVideoProfileLevel(mMp4FileHandle, 0x7F);
            mVideoFrameRate = framerate;
            return true;
        }
    } else {
        LOGE(" %s MP4文件句柄无效", __FUNCTION__);
        return false;
    }
    return false;
}

bool Mp4Muxer::SetupVideoTrack(ps_t& sps, ps_t& pps, double frameRate) {
    if (mMp4FileHandle != MP4_INVALID_FILE_HANDLE) {
        MP4SetTimeScale(mMp4FileHandle, TICKS);
        int32_t width = 0 , height = 0, framerate = 0;

        h264_decode_sps(sps.data, sps.count, &width, &height, &framerate);//有时sps中获取不到framerate，所以不用这里获取的framerate
        if (framerate <= 0){
            framerate = (int)(frameRate + 0.5);
        }
        LOGI("TICKS:%lu, width:%d, heigth:%d, framerate:%d, dFrameRate:%f", TICKS, width, height, framerate, frameRate);
        LOGD("SPS.data[1]:%u, SPS.data[2]:%u, SPS.data[3]:%u", sps.data[1], sps.data[2], sps.data[3]);
        mVideoTrackId = MP4AddH264VideoTrack(mMp4FileHandle, TICKS, TICKS / framerate, width,
                                             height,
                                             sps.data[1], //sps[1] AVCProfileIndication
                                             sps.data[2], //sps[2] profile_compat
                                             sps.data[3], //sps[3] AVCLevelIndication
                                             3); // 4 bytes length before each NAL unit

        if (mVideoTrackId == MP4_INVALID_TRACK_ID) {
            LOGE(" %s 增加H264Track失败", __FUNCTION__);
        } else {
            MP4AddH264SequenceParameterSet(mMp4FileHandle, mVideoTrackId, sps.data, sps.count);
            MP4AddH264PictureParameterSet(mMp4FileHandle, mVideoTrackId, pps.data, pps.count);
            MP4SetVideoProfileLevel(mMp4FileHandle, 0x7F);
            mVideoFrameRate = framerate;
            return true;
        }
    } else {
        LOGE(" %s MP4文件句柄无效", __FUNCTION__);
        return false;
    }
    return false;
}

bool Mp4Muxer::WriteVideoSample(const uint8_t *H264Data , int64_t iSize , int64_t iTimestamp) {
    if (mMp4FileHandle == MP4_INVALID_FILE_HANDLE) {
        LOGE(" %s MP4文件句柄无效", __FUNCTION__);
        return false;
    }

    if (mVideoTrackId == MP4_INVALID_TRACK_ID) {
        LOGE(" %s H264 Track无效", __FUNCTION__);
        return false;
    }

    //获得NAL单元类型
    int iNALType = get_nal_type((void *) H264Data, iSize);
    bool idr = iNALType == 5;

    MP4Duration ttt = MP4_INVALID_DURATION;
#if 0
    if (mLastVideoTimestampUs != 0) {
        ttt = (iTimestamp - mLastVideoTimestampUs) * TICKS / 1000000LL;
    }
#endif

    int iInlen = iSize;
    const unsigned char *pIn = H264Data;
    int iOutLen = 0;
    const unsigned char *pOut = NULL;
    unsigned char *pRealData = (unsigned char*)calloc(1,iSize << 1);
    if(NULL == pRealData){
        LOGE(" %s 分配缓冲失败", __FUNCTION__);
        return false;
    }
    int nRealDataSize = 0;
    do {
        int nal_start = 0;
        int nal_end = 0;
        iOutLen = find_nal_units(pIn, iInlen, &nal_start, &nal_end);
        if (iOutLen <= 0) {
            break;
        }
        pOut = pIn + nal_start;
        if (pOut != NULL) {
            unsigned char naltype = get_nal_type((void *) pOut, iOutLen);
            int iStartCodeNum = h24_get_start_bytes(pOut, iOutLen);
            pOut += iStartCodeNum;
            iOutLen -= iStartCodeNum;

           if (naltype == NALU_TYPE_SPS){
               // MP4AddH264SequenceParameterSet(mMp4FileHandle, mVideoTrackId, pout, outlen);
            } else if (naltype == NALU_TYPE_PPS){
               // MP4AddH264PictureParameterSet(mMp4FileHandle, mVideoTrackId, pout, outlen);
            } else if (naltype == NALU_TYPE_SEI){

            } else {
                uint32_t *p = (uint32_t *) (pRealData + nRealDataSize);
                *p = htonl(iOutLen);//大端,去掉头部四个字节
                nRealDataSize += 4;

                memcpy(pRealData + nRealDataSize, pOut, iOutLen);
                nRealDataSize += iOutLen;
            }

            iInlen = iInlen - iOutLen - iStartCodeNum;
            pIn = pOut + iOutLen;
        }
    } while (true);


    mLastVideoTimestampUs = iTimestamp;
    bool b = MP4WriteSample(mMp4FileHandle, mVideoTrackId, pRealData, nRealDataSize, ttt, 1, idr);
    mVideoTrackId = b ? mVideoTrackId : MP4_INVALID_TRACK_ID;
    mFrameCounter++;

    if (pRealData) {
        free(pRealData);
    }

    return true;

}

void Mp4Muxer::parse_sps_pps(uint8_t *buffer, int64_t count, ps_t &sps, ps_t &pps) {
    //ALOGD("parse_sps_pps buffer=%p,count=%lld",buffer,count);

    int64_t i = 0;
    int64_t s_start = -1;
    int64_t s_end = -1;

    /*
    for(i=0;i<count;i++){
        ALOGD("parse_sps_pps buffer[%lld]=%02x",i,buffer[i]);
    }
    */

    for (i = 0; i < count - 4; i++) {
        if (buffer[i] == 0x00 && buffer[i + 1] == 0x00 && buffer[i + 2] == 0x00 &&
            buffer[i + 3] == 0x01) {
            if (s_start == -1) {
                s_start = i + 4;
            } else {
                s_end = i;
            }
            i = i + 4;
        }
    }
    //ALOGD("parse_sps_pps s_start=%lld",s_start);
    if (s_start >= 0) {
        if (s_end < 0) {
            pps.count = 0;
        } else {
            pps.count = count - s_end - 4;
            for (i = 0; i < pps.count; i++) {
                pps.data[i] = buffer[s_end + 4 + i];
            }
        }
        s_end = s_end < 0 ? count : s_end;
        sps.count = s_end - s_start;
        for (i = 0; i < sps.count; i++) {
            sps.data[i] = buffer[s_start + i];
        }
    } else {
        sps.count = 0;
        pps.count = 0;
    }
}
