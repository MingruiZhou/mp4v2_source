#ifndef _TOOL_H
#define _TOOL_H
#ifdef __cplusplus
extern "C" {
#endif
//H264定义的类型 values for nal_unit_type
typedef enum {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
#if (MVC_EXTENSION_ENABLE)
    NALU_TYPE_PREFIX   = 14,
    NALU_TYPE_SUB_SPS  = 15,
    NALU_TYPE_SLC_EXT  = 20,
    NALU_TYPE_VDRD     = 24  // View and Dependency Representation Delimiter NAL Unit
#endif
} enNALUType;

int find_nal_units(const unsigned char *buf, int buf_size, int *nal_start, int *nal_end);

/* 十六进制转十进制
 *
 */
int hex2int(int hexValue);

/* 获取绝对时间(单位毫秒)
 *
 */
unsigned long long GetCurSysTime();

void mysleep(unsigned int milliseconds);

int get_nal_type(const void *p, int len);

#ifdef __cplusplus
}
#endif
#endif //_TOOL_H
