#include <jni.h>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <include/mp4v2/mp4v2.h>
#include "MG_log.h"
//#include "libmp4v2-android/mp4v2.h"

unsigned char sps[64],pps[64];
int spslen = 0,ppslen = 0;

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
    pFile = fopen("out.h264","wb");

    while(nReadIndex < totalFrame)
    {
        nReadIndex ++;
        //LOGD("nReadIndex:%d\n",nReadIndex);
        MP4ReadSample(oMp4File,VTrackId,nReadIndex,&pData,&nSize,&pStartTime,&pDuration,&pRenderingOffset,&pIsSyncSample);

        //IDR֡ 帧，写入sps pps先
        if(pIsSyncSample)
        {
            fwrite(NAL,4,1,pFile);
            fwrite(sps,spslen,1,pFile);

            fwrite(NAL,4,1,pFile);
            fwrite(pps,ppslen,1,pFile);

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
    int videoindex = -1,audioindex = -1;
    uint32_t numSamples;
    //uint32_t timescale;
    //uint64_t duration;

    if (!oMp4File)
    {
        LOGE("Read error....%s\r\n",sMp4file);
        return -1;
    }

    LOGI("----------------- open file successed!!!  -------------------------");

    MP4TrackId trackId = MP4_INVALID_TRACK_ID;
    uint32_t numTracks = MP4GetNumberOfTracks(oMp4File,NULL,0);
    LOGI("numTracks:%d\n",numTracks);

    for (i = 0; i < numTracks; i++)
    {
        trackId = MP4FindTrackId(oMp4File, i,NULL,0);
        const char* trackType = MP4GetTrackType(oMp4File, trackId);
        if (MP4_IS_VIDEO_TRACK_TYPE(trackType))
        {
            //LOGI("[%s %d] trackId:%d\r\n",__FUNCTION__,__LINE__,trackId);
            videoindex= trackId;

            //duration = MP4GetTrackDuration(oMp4File, trackId );
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

    //需要mp4close 否则在嵌入式设备打开mp4上多了会内存泄露挂掉.
    MP4Close(oMp4File, 0);
    return 0;
}

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

extern "C" JNIEXPORT jstring

JNICALL
Java_google_mp4v2_1github_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "/sdcard/TTT.mp4";
    testOpenFile(hello.c_str());
    openmp4file(hello.c_str());
    return env->NewStringUTF(hello.c_str());
}