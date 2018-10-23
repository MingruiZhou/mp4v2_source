#ifndef _MG_LOG_H_
#define _MG_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OS_ANDROID
#ifdef OS_ANDROID
#include <android/log.h>
#include <stdint.h>
#include <cstring>
#ifndef LOG_TAG
//为什么'/'适用于.h文件， '\\'适用于.cpp文件呢
#define LOG_TAG (NULL == strrchr(__FILE__, '\\') ? __FILE__ : (char *)(strrchr(__FILE__, '\\') + 1))
#endif
#define MG_LOG_UNKNOWN (0)   //    ANDROID_LOG_UNKNOWN = 0,
#define MG_LOG_DEFAULT (1)   //    ANDROID_LOG_DEFAULT,
#define MG_LOG_VERBOSE (2)   //    ANDROID_LOG_VERBOSE,
#define MG_LOG_DEBUG   (3)   //    ANDROID_LOG_DEBUG,
#define MG_LOG_INFO    (4)   //    ANDROID_LOG_INFO,
#define MG_LOG_WARN    (5)   //    ANDROID_LOG_WARN,
#define MG_LOG_ERROR   (6)   //    ANDROID_LOG_ERROR,
#define MG_LOG_FATAL   (7)   //    ANDROID_LOG_FATAL,
#define MG_LOG_SILENT  8      //    ANDROID_LOG_SILENT,

#define JLZ_DBG_ENABLE -1
#define LOGD(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_DEBUG) \
                        { \
                            std::string TAG = "(";\
                            TAG += LOG_TAG;\
                            TAG += ":";\
                            TAG += std::to_string(__LINE__);\
                            TAG += ")";\
                            __android_log_print(ANDROID_LOG_DEBUG, TAG.c_str(), "[%s]" fmt, __FUNCTION__, ##__VA_ARGS__);\
                        }

#define LOGI(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_INFO) \
                        { \
                            std::string TAG = "(";\
                            TAG += LOG_TAG;\
                            TAG += ":";\
                            TAG += std::to_string(__LINE__);\
                            TAG += ")";\
                             __android_log_print(ANDROID_LOG_INFO, TAG.c_str(), "[%s]" fmt, __FUNCTION__, ##__VA_ARGS__);\
                        }

#define LOGW(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_WARN) \
                        { \
                            std::string TAG = "(";\
                            TAG += LOG_TAG;\
                            TAG += ":";\
                            TAG += std::to_string(__LINE__);\
                            TAG += ")";\
                            __android_log_print(ANDROID_LOG_WARN, TAG.c_str(), "[%s]" fmt, __FUNCTION__, ##__VA_ARGS__);\
                        }

#define LOGE(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_ERROR) \
                        { \
                            std::string TAG = "(";\
                            TAG += LOG_TAG;\
                            TAG += ":";\
                            TAG += std::to_string(__LINE__);\
                            TAG += ")";\
                            __android_log_print(ANDROID_LOG_ERROR, TAG.c_str(), "[%s]" fmt, __FUNCTION__, ##__VA_ARGS__);\
                        }

#define LOGV(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_VERBOSE) \
                        { \
                            std::string TAG = "(";\
                            TAG += LOG_TAG;\
                            TAG += ":";\
                            TAG += std::to_string(__LINE__);\
                            TAG += ")";\
                            __android_log_print(ANDROID_LOG_FATAL, TAG.c_str(), "[%s]" fmt, __FUNCTION__, ##__VA_ARGS__);\
                        }

#define LOGMP4V2(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_VERBOSE) \
                        { \
                            __android_log_print(ANDROID_LOG_FATAL, LOG_TAG, "" fmt, ##__VA_ARGS__);\
                        }

#elif defined OS_JLZ
#define MG_LOG_UNKNOWN (-8)   //    ANDROID_LOG_UNKNOWN = 0,
#define MG_LOG_DEFAULT (-7)   //    ANDROID_LOG_DEFAULT,
#define MG_LOG_VERBOSE (-6)   //    ANDROID_LOG_VERBOSE,
#define MG_LOG_DEBUG   (-5)   //    ANDROID_LOG_DEBUG,
#define MG_LOG_INFO    (-4)   //    ANDROID_LOG_INFO,
#define MG_LOG_WARN    (-3)   //    ANDROID_LOG_WARN,
#define MG_LOG_ERROR   (-2)   //    ANDROID_LOG_ERROR,
#define MG_LOG_FATAL   (-1)   //    ANDROID_LOG_FATAL,
#define MG_LOG_SILENT  0      //    ANDROID_LOG_SILENT,

#define LOGD(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_DEBUG) \
						{ \
							printc(ESC_ATR_COLOR_DEFAULT);\
							printc("[%s:%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
							printc("\n");\
						}

#define LOGI(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_INFO) \
						{ \
							printc(ESC_FG_COLOR_BLUE);\
							printc("[%s:%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
							printc(ESC_ATR_COLOR_DEFAULT"\n");\
						}

#define LOGW(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_WARN) \
						{ \
							printc(ESC_FG_COLOR_YELLOW);\
							printc("[%s:%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
							printc(ESC_ATR_COLOR_DEFAULT"\n");\
						}

#define LOGE(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_ERROR) \
						{ \
							printc(ESC_BG_COLOR_RED);\
							printc("[%s:%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
							printc(ESC_ATR_COLOR_DEFAULT"\n");\
						}

#define LOGV(fmt, ...)  if (JLZ_DBG_ENABLE <=  MG_LOG_VERBOSE) \
						{ \
							printc(ESC_BG_COLOR_RED);\
							printc("[%s:%d]"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__);\
							printc(ESC_ATR_COLOR_DEFAULT"\n");\
						}
#endif

#ifdef __cplusplus
};
#endif


#endif //_MG_LOG_H_
