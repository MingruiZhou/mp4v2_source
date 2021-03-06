# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#CMAKE_CURRENT_SOURCE_DIR = C:/Users/ouyangjie/AndroidStudioProjects/mp4v2_source/app
message("-------------- ${CMAKE_CURRENT_SOURCE_DIR} ---------------")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libyuv-android)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libmp4v2-android)
#暂时不添加oRTP的库，因为使用的oRTP源码依赖的其它库
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libortp-android)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/liblive555-android)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libmp4v2-android
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libmp4v2-android/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libmp4v2-android/include/mp4v2
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/libortp-android/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/MP4Parse)

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp
             src/main/cpp/MP4Parse/MP4Parse.cpp
             src/main/cpp/MP4Parse/Mp4Muxer.cpp
             src/main/cpp/MP4Parse/Mp4TimeLapseMuxer.cpp
             src/main/cpp/MP4Parse/tool.c
             src/main/cpp/MP4Parse/sps_decode.c)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       yuv
                       mp4v2
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )