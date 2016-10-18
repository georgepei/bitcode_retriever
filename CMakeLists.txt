cmake_minimum_required(VERSION 3.5)
project(bitcode_retriever)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#add_definitions(-I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include/libxml2/)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lxar -lxml2")

#add_definitions(-lxar -lxml2)

set(SOURCE_FILES
    macho_reader.c
    macho_reader.h
    macho_retriever.c
    macho_retriever.h
    macho_util.c
    macho_util.h
    main.c
        WinList.c
        WinList.h)

add_executable(bitcode_retriever ${SOURCE_FILES})