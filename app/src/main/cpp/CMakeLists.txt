# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -v")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -Wall -v")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(jni_libs_dir ${CMAKE_SOURCE_DIR}/../jniLibs)

# includes .h head files.
include_directories(${jni_libs_dir}/includes)

set(
        ffmpeg_libs_names  # List variable name
        avutil avformat avcodec swresample swscale wkmedia # Values in the list
)

foreach (ffmpeg_lib_name ${ffmpeg_libs_names})
    add_library(
            ${ffmpeg_lib_name}
            SHARED
            IMPORTED)
    set_target_properties(
            ${ffmpeg_lib_name}
            PROPERTIES
            IMPORTED_LOCATION
            ${jni_libs_dir}/${ANDROID_ABI}/lib${ffmpeg_lib_name}.so)
endforeach ()

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).

#        core_player.c
#        test_player.c
#        Muxer.c
#        NALUParser.c
        native-lib.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library(android-lib android)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android-lib}
        ${ffmpeg_libs_names}
        )