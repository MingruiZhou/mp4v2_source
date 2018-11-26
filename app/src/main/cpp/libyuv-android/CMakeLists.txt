# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

SET ( ly_cmake_dir ${CMAKE_CURRENT_SOURCE_DIR} )  #cmake所在的目录

SET ( ly_base_dir   ${CMAKE_CURRENT_SOURCE_DIR})
SET ( ly_src_dir    ${ly_base_dir}/source)
SET ( ly_inc_dir    ${ly_base_dir}/include )
SET ( ly_lib_name   yuv )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fexceptions -Wno-write-strings -O2 -DHAVE_SOCKLEN_T -DHAVE_STRUCT_IOVEC")

message("------------------------------Cmake build ${ly_lib_name}-----------------------------")
message("project source dir==>${PROJECT_SOURCE_DIR}")
message("Cmake file dir==>${CMAKE_CURRENT_SOURCE_DIR}")

#查找目录下所有的源文件，并保存在指定变量ly_source_files中,遗憾的是它不能像File递归查找

aux_source_directory( source ly_source_files)
# FILE ( GLOB_RECURSE   ly_source_files ./src/*.cpp )
#LIST ( SORT    ly_source_files )

#排序之后的文件
message("-----------------  The following is the sorted source file   --------------------")
message("${ly_source_files}")

foreach(_var ${ly_source_files})
   message("${_var}")
   if(_var MATCHES "_win32.cpp")
         list(APPEND  filter_source_cpp ${_var})
   endif()
endforeach()

foreach(_var ${filter_source_cpp})
    message("remove win32==>${_var}")
    # 切记此处remove_item 的list不能  ${}  求值
    list(REMOVE_ITEM ly_source_files ${_var})
endforeach()

message("----------------   file sorted END and delete win32 file END   -----------------------")

if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
set_property(SOURCE ${ly_source_files} APPEND_STRING PROPERTY COMPILE_FLAGS " -mfpu=neon")
endif ()

# include目录  AFTER OR BEFORE 表示添加在之前还是之后,SYSTEM表示系统包含目录
# https://cmake.org/cmake/help/latest/command/include_directories.html
include_directories( BEFORE ${ly_inc_dir} )

#-------------------------------------------------------------------------------------------
# this creates the static library (.a)
#ADD_LIBRARY ( ${ly_lib_name} STATIC ${ly_source_files} )

# this creates the shared library (.so)
ADD_LIBRARY    ( ${ly_lib_name} SHARED ${ly_source_files} )

#可以输出同名的  动、静态库
#SET_TARGET_PROPERTIES  ( ${ly_lib_name} PROPERTIES OUTPUT_NAME "${ly_lib_name}" )
## SET_TARGET_PROPERTIES    ( ${ly_lib_name} PROPERTIES PREFIX "lib" )

#cmake在构建完一个target构建下一个target时会clean掉上一个使用此target名字的库
set_target_properties ( ${ly_lib_name} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#set_target_properties ( ${ly_lib_name} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

#动态库版本号
##set_target_properties ( ${ly_lib_name} properties VERSION 1.0 SOVERSION 1)
message("-------------------buile ${ly_lib_name} END  -------------------")