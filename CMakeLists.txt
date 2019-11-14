cmake_minimum_required(VERSION 3.2)
project(rtsplib VERSION 1.0.0 LANGUAGES CXX C)

# **********************************************************************************
#                                                                                   #
# Copyright (c) 2019,                                                               #
# Research group MITI                                                               #
# Technical University of Munich                                                    #
#                                                                                   #
# All rights reserved.                                                              #
# Kevin Yu - kevin.yu@tum.de                                                        #
#                                                                                   #
# Redistribution and use in source and binary forms, with or without                #
# modification, are restricted to the following conditions:                         #
#                                                                                   #
#  * The software is permitted to be used internally only by the research group     #
#    MITI and CAMPAR and any associated/collaborating groups and/or individuals.    #
#  * The software is provided for your internal use only and you may                #
#    not sell, rent, lease or sublicense the software to any other entity           #
#    without specific prior written permission.                                     #
#    You acknowledge that the software in source form remains a confidential        #
#    trade secret of the research group MITI and therefore you agree not to         #
#    attempt to reverse-engineer, decompile, disassemble, or otherwise develop      #
#    source code for the software or knowingly allow others to do so.               #
#  * Redistributions of source code must retain the above copyright notice,         #
#    this list of conditions and the following disclaimer.                          #
#  * Redistributions in binary form must reproduce the above copyright notice,      #
#    this list of conditions and the following disclaimer in the documentation      #
#    and/or other materials provided with the distribution.                         #
#  * Neither the name of the research group MITI nor the names of its               #
#    contributors may be used to endorse or promote products derived from this      #
#    software without specific prior written permission.                            #
#                                                                                   #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND   #
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     #
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE            #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR   #
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    #
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      #
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND       #
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS     #
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                      #
#                                                                                   #
# ***********************************************************************************

# XXX these flags only work on linux !!!
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -fPIC")

option(BUILD_SHARED_LIBS "Build shared library" ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

include(GNUInstallDirs)
#################################################################################################
# RTSP Server and Client
#################################################################################################

set(RTSP_COMMON_HDRS
        ${PROJECT_SOURCE_DIR}/include/rtsplib/rtsplib_common.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/rtsplib_config.h
        )

set(RTSP_CLIENT_HDRS
        ${PROJECT_SOURCE_DIR}/include/rtsplib/ClientPipeRTSP.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/client/sdp.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/client/RtspPlayer.h
        )

set(RTSP_CLIENT_SRCS
        ${PROJECT_SOURCE_DIR}/src/client/sdp.c
        ${PROJECT_SOURCE_DIR}/src/client/RtspPlayer.cpp
        ${PROJECT_SOURCE_DIR}/src/ClientPipeRTSP.cpp
        )

set(RTSP_SERVER_HDRS
        ${PROJECT_SOURCE_DIR}/include/rtsplib/ServerPipeRTSP.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Acceptor.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/BufferReader.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/BufferWriter.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Channel.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/EpollTaskScheduler.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/EventLoop.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/log.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Logger.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/MemoryManager.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/NetInterface.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Pipe.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/RingBuffer.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/SelectTaskScheduler.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Socket.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/SocketUtil.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/TaskScheduler.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/TcpConnection.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/TcpServer.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/TcpSocket.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/ThreadSafeQueue.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Timer.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/net/Timestamp.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/AACSource.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/G711ASource.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/H264Parser.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/H264Source.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/H265Source.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/media.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/MediaSession.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/MediaSource.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/rtp.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/RtpConnection.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/rtsp.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/RtspConnection.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/RtspMessage.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/RtspPusher.h
        ${PROJECT_SOURCE_DIR}/include/rtsplib/server/xop/RtspServer.h
        )

set(RTSP_SERVER_SRCS
        ${PROJECT_SOURCE_DIR}/src/ServerPipeRTSP.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/Acceptor.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/BufferReader.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/BufferWriter.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/EpollTaskScheduler.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/EventLoop.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/Logger.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/MemoryManager.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/NetInterface.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/Pipe.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/SelectTaskScheduler.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/SocketUtil.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/TaskScheduler.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/TcpConnection.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/TcpServer.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/TcpSocket.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/Timer.cpp
        ${PROJECT_SOURCE_DIR}/src/server/net/Timestamp.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/AACSource.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/G711ASource.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/H264Parser.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/H264Source.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/H265Source.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/MediaSession.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/RtpConnection.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/RtspConnection.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/RtspMessage.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/RtspPusher.cpp
        ${PROJECT_SOURCE_DIR}/src/server/xop/RtspServer.cpp
        )


add_library(rtsplib ${RTSP_CLIENT_SRCS} ${RTSP_SERVER_SRCS} ${RTSP_COMMON_HDRS} ${RTSP_CLIENT_HDRS} ${RTSP_SERVER_HDRS})

set_property(TARGET rtsplib PROPERTY CXX_STANDARD 11)
set_target_properties(rtsplib PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(rtsplib PRIVATE
        CONAN_PKG::spdlog
        )

target_include_directories(rtsplib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/include
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${PROJECT_BINARY_DIR}/src
        )

if(BUILD_SHARED_LIBS)
    set_target_properties(rtsplib PROPERTIES COMPILE_DEFINITIONS "RTSPLIB_DLL")
endif()

# only valid on linux
SET_TARGET_PROPERTIES(rtsplib PROPERTIES COMPILE_FLAGS "-fPIC")

install(TARGETS rtsplib DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_subdirectory(tests)