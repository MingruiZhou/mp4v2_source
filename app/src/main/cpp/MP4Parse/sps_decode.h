#ifndef __SPS_DECODE_H__
#define __SPS_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

int h24_get_start_bytes(const unsigned char *buf,unsigned int nLen);
int h264_decode_sps(const unsigned char * buf,unsigned int nLen,int* width,int* height,int* fps);

#ifdef __cplusplus
}
#endif
#endif