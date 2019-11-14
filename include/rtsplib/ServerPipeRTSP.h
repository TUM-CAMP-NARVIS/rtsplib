#pragma once

#include "rtsplib/rtsplib_common.h"
#include "rtsplib/server/xop/RtspServer.h"
#include "rtsplib/server/net/NetInterface.h"
#include <rtsplib/server/net/EventLoop.h>
#include <thread>
#include <mutex>

#include "rtsplib_config.h"

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

namespace rtsplib
{
  class RTSPLIB_EXPORT ServerPipeRTSP
  {
    public:
      ServerPipeRTSP(std::string ipAddress, int port);
      ~ServerPipeRTSP() = default;

      bool startServer(int width, int height, int bytesPerPixel);

      bool sendFrame(u_char* data, size_t size, uint32_t ts = 0);

      void stopServer();

    private:
      bool init_Loop(int _width, int _height, int _bytesPerPixel);

      ssize_t startCodePosition(const u_char* buffer, int maxSize, size_t offset);
      void get_NalPackage(u_char* buffer, int maxSize, int *head, int *tail, int *length);

      std::string m_ipAddress;

      bool m_isRunning{false};
      int m_port{0};
      int m_width{0};
      int m_height{0};
      int m_bytesPerPixel{0};

      xop::MediaSessionId m_sessionId;
      std::shared_ptr<xop::EventLoop> m_eventLoop;
      std::unique_ptr<xop::RtspServer> m_server;
      std::thread m_rtspThread;
      static std::mutex m_coutMutex;

      Timer m_timer;

  };
}
