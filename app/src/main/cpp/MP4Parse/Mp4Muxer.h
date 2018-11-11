#ifndef _MP4_WRITER_H_
#define _MP4_WRITER_H_


#include <arpa/inet.h>
#include <mp4v2.h>
#include <string>

using namespace std;
#define SPS_PPS_DATA_MAX_LEN  (128)
typedef struct {
	int32_t count ;
	uint8_t data[SPS_PPS_DATA_MAX_LEN];
} ps_t ;

class Mp4Muxer
{
public :
	Mp4Muxer(const char* filename);
	virtual ~Mp4Muxer();

	bool SetupVideoTrack(int32_t width , int32_t height , int32_t framerate , uint8_t* sps_pps_data , int64_t data_count);
	bool SetupAudioTrack(int32_t sampleRate,int32_t fixedDuration ,const uint8_t* pConfig,uint32_t  configSize);
	bool SetupVideoTrack(ps_t& sps, ps_t& pps, double frameRate);

	bool WriteVideoSample(const uint8_t *H264Data , int64_t iSize , int64_t iTimestamp);
	bool WriteAudioSample(uint8_t *adts_aac , int64_t size , int64_t timestamp);


	string GetFileName();

    uint64_t GetVideoDuration();

	uint64_t GetAudioDuration();
private:
	static void parse_sps_pps(uint8_t* buffer , int64_t count, ps_t &sps , ps_t &pps);
	void CheatFrameRate();
    bool Check();
	
	MP4FileHandle mMp4FileHandle;
	MP4TrackId mVideoTrackId;
	MP4TrackId mAudioTrackId;
	
	int64_t mLastVideoTimestampUs;
	int64_t mLastAudioTimestampUs;
	int32_t mFrameCounter;
	
	string mFilename ;
	int32_t mVideoFrameRate;

	int mSampleRate;
};

#endif
