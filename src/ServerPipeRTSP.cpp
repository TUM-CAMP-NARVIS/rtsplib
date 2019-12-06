#include "rtsplib/rtsplib_common.h"
#include "rtsplib/ServerPipeRTSP.h"

#include "spdlog/spdlog.h"

/* **********************************************************************************
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
*************************************************************************************/

using namespace std;
using namespace rtsplib;

ServerPipeRTSP::ServerPipeRTSP(std::string _ipAddress, int _port)
: m_ipAddress(std::move(_ipAddress)), m_port(_port)
{
}

bool ServerPipeRTSP::startServer(int width, int height, int bytesPerPixel) {
    m_width = width;
    m_height = height;
    m_bytesPerPixel = bytesPerPixel;
    return init_Loop(width, height, bytesPerPixel);
}

bool ServerPipeRTSP::init_Loop(int _width, int _height, int _bytesPerPixel)
{

    // RTSP ############################################################
    m_rtspThread = thread([&, _width, _height, _bytesPerPixel]() {
        int clients = 0;
        std::string ip = m_ipAddress; // xop::NetInterface::getLocalIPAddress();
        std::string rtspUrl;

        m_eventLoop.reset(new xop::EventLoop());
        m_server.reset(new xop::RtspServer(m_eventLoop.get(), ip, m_port));

        xop::MediaSession *session = xop::MediaSession::createNew("live");
        rtspUrl = "rtsp://" + ip + ":" + std::to_string(m_port) + "/" + session->getRtspUrlSuffix();

        session->addMediaSource(xop::channel_0, xop::H264Source::createNew());
        session->setMediaDescribeSDPAddon("a=x-dimensions:" + std::to_string(_width) + "," + std::to_string(_height) + "," + std::to_string(_bytesPerPixel) + "\n");

        session->setNotifyCallback([&clients, &rtspUrl](xop::MediaSessionId sessionId, uint32_t numClients) {
            clients = numClients;
            spdlog::info("RTSP Server {0} is online with {1} clients", rtspUrl, clients);
        });

        m_sessionId = m_server->addMediaSession(session);
        spdlog::info("RTSP Server URL: {0}", rtspUrl);

        m_eventLoop->loop();
    });
    // END RTSP ########################################################

    m_isRunning = true;
    return true;
}

bool ServerPipeRTSP::sendFrame(unsigned char* data, int size, uint32_t timestamp)
{
    if (!m_isRunning) {
        spdlog::warn("Server is not running!");
        return false;
    }

    // RTSP ############################################################
    int tail = 0;
    while (tail < size)
    {
        int length;
        int head;
        get_NalPackage(data, size, &head, &tail, &length);

        xop::AVFrame videoFrame = {0};
        videoFrame.type = 0;
        videoFrame.size = length;
        videoFrame.timestamp = timestamp != 0 ? timestamp : xop::H264Source::getTimeStamp();
        videoFrame.buffer = (uint8_t*)&data[head];

        bool all_zero{true};
        for (size_t i=0; i<length;i++){
            if (videoFrame.buffer[i] != 0)
                all_zero = false;

        }
        if (all_zero)
            spdlog::warn("all bytes are zero ...");

        m_server->pushFrame(m_sessionId, xop::channel_0, videoFrame);
    }

    // END RTSP ############################################################

    return true;
}

int ServerPipeRTSP::startCodePosition(const unsigned char* buffer, int maxSize, int offset)
{
	assert(offset < maxSize);

    /* Simplified Boyer-Moore, inspired by gstreamer */
    while (offset < maxSize - 2)
    {
        switch((int)buffer[offset + 2])
        {
            case 1:
            {
                if ((int)buffer[offset] == 0 && (int)buffer[offset + 1] == 0 )
                {
                    return offset;
                }
                offset += 3;
                break;
            }
            case 0:
                offset++;
            break;
            default:
                offset += 3;
        }
    }

    return -1;
}

void ServerPipeRTSP::get_NalPackage(unsigned char* buffer, int maxSize, int *head, int *tail, int *length)
{
    int start = startCodePosition(buffer, maxSize, *tail);
    int end = startCodePosition(buffer, maxSize, start + 1);

    if(end < 0) end = maxSize;

    int packageLength = end - start;

    *head = start;
    *tail = end;
    *length = packageLength;
}

void ServerPipeRTSP::stopServer()
{
    if(!m_isRunning) return;

    m_isRunning = false;
    m_eventLoop->quit();
    m_rtspThread.join();
}
